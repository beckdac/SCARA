/*

This file is licensed under the X11 license:

Copyright (C) 2013 Andreas Ehliar

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL ANDREAS EHLIAR BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Andreas Ehliar shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from Andreas Ehliar.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>

// General purpose error message
// A real system would probably have a better error handling method...
static void panic(char *message)
{
        fprintf(stderr,"Fatal error: %s\n", message);
        exit(1);
}

struct stepper{
        // Used for synchronization between user thread and
        // the real time thread
        sem_t sem;
        sem_t rt_sem;

        int initial_delay;
        int target_delay;

        // File descriptors for our GPIO pins
        FILE *fp[4];

        // Handle commands from the user task
        int getrpm;
        int num_steps; // How many times have we stepped?
        int doexit;

        // Dummy do work thread
        char *work_area;
};




void *stepper_rt_thread(void *arg);

void *user_interface_thread(void *arg)
{
        struct stepper *step = (struct stepper *) arg;
        while(1){
                char buf[80];
                printf("Enter command (q, d or p): ");
                fgets(buf,79,stdin);
                if(buf[0] == 'q'){
                        // Exit
                        step[0].doexit = 1;
                        step[1].doexit = 1;
                        sem_post(&step[0].sem); // Notify RT thread
                        sem_post(&step[1].sem); // Notify RT thread
                        // No need to wait for acknowledgment before exit...
                        pthread_exit(0);
                }else if(buf[0] == 'd'){
                        int delay;
                        int stepno;
                        if(sscanf(buf,"d %d %d", &stepno, &delay) == 2){
                                step[stepno].target_delay = delay;
                                sem_post(&step[stepno].sem);
                                sem_wait(&step[stepno].rt_sem);
                        }
                }else if(buf[0] == 'p'){
                        step[0].getrpm = 1;
                        step[1].getrpm = 1;
                        sem_post(&step[0].sem);
                        sem_post(&step[1].sem);
                        sem_wait(&step[0].rt_sem);
                        sem_wait(&step[1].rt_sem);
                        printf("Current stepval for stepper 0 is %d\n",step[0].num_steps);
                        printf("Current stepval for stepper 1 is %d\n",step[1].num_steps);
                }
        }
}


// Initialize a GPIO pin in Linux using the sysfs interface
FILE *init_gpio(int gpioport)
{
        // Export the pin to the GPIO directory
        FILE *fp = fopen("/sys/class/gpio/export","w");
        fprintf(fp,"%d",gpioport);
        fclose(fp);

        // Set the pin as an output
        char filename[256];
        sprintf(filename,"/sys/class/gpio/gpio%d/direction",gpioport);
        fp = fopen(filename,"w");
        if(!fp){
                panic("Could not open gpio file");
        }
        fprintf(fp,"out");
        fclose(fp);

        // Open the value file and return a pointer to it.
        sprintf(filename,"/sys/class/gpio/gpio%d/value",gpioport);
        fp = fopen(filename,"w");
        if(!fp){
                panic("Could not open gpio file");
        }
        return fp;
}

// Given a FP in the stepper struct, set the I/O pin
// to the specified value. Uses the sysfs GPIO interface.
void setiopin(struct stepper *step, int pin, int val)
{
        fprintf(step->fp[pin],"%d\n",val);
        fflush(step->fp[pin]);
}

// Initialize the stepper struct
void init_stepper(struct stepper *step, int port1, int port2, int port3, int port4)
{
        // Make sure general purpose I/O is visible

        step->fp[0] = init_gpio(port1);
        step->fp[1] = init_gpio(port2);
        step->fp[2] = init_gpio(port3);
        step->fp[3] = init_gpio(port4);

        sem_init(&step->sem,0,0);
        sem_init(&step->rt_sem,0,0);
        step->initial_delay = 1000000;
        step->target_delay = 1000000;
        step->getrpm = 0;

        step->doexit = 0;
        
        step->work_area = malloc(1024*1024);
}



// Demo program for running two steppers at the same time connected to
// the Raspberry PI platform.
int main(int argc, char **argv)
{
        struct stepper step[2];
        init_stepper(&step[0],14,15,17,18); // Pins for stepper 1
        init_stepper(&step[1],25,23,24,22); // Pins for stepper 2

        // Ensure that all memory that we allocate is locked
        // to prevent swapping!
        if(mlockall(MCL_FUTURE|MCL_CURRENT)){
                fprintf(stderr,"WARNING: Could not lock memory with mlockall()\n");
        }

        // Create our main threads
        pthread_t user_thread;
        pthread_t stepper_thread[2];

        pthread_create(&user_thread, NULL, (void *)(user_interface_thread), (void *) &step[0]);
        pthread_create(&stepper_thread[0], NULL, stepper_rt_thread, (void *) &step[1]);
        pthread_create(&stepper_thread[1], NULL, stepper_rt_thread, (void *) &step[0]);


        // Waits for threads to exit
        pthread_join(user_thread, NULL);
        pthread_join(stepper_thread[0], NULL);
        pthread_join(stepper_thread[1], NULL);

        printf("\nAll threads have exited\n\n");
        return 0;
}

// Adds "delay" nanoseconds to timespecs and sleeps until that time
static void sleep_until(struct timespec *ts, int delay)
{
        
        ts->tv_nsec += delay;
        if(ts->tv_nsec > 1000*1000*1000){
                ts->tv_nsec -= 1000*1000*1000;
                ts->tv_sec++;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, ts,  NULL);
}



/* The thread that is responsible for actually updating the GPIO pins
 * connected to the steppers */
void * stepper_rt_thread(void *arg)
{

        struct stepper *step = (struct stepper *) arg;
        struct timespec ts;

        // Ensure that we have real time priority set
        struct sched_param sp;
        sp.sched_priority = 30;
        if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)){
                fprintf(stderr,"WARNING: Failed to set stepper thread to realtime priority\n");
        }

        // Initialize timespec from the monotonically increasing clock source
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_nsec = 0;
        int delay = step->initial_delay;
        int target_delay = step->target_delay;

        int num_steps = 0;
        while(1){

                setiopin(step,0,1);
                num_steps++;
                // Check for commands from user interface thread
                if(!sem_trywait(&step->sem)){
                        target_delay = step->target_delay;
                        // Note: Do not decrement step->sem here. 
                        // The user interface is responsible for
                        // incrementing step->sem when it has
                        // a command for the real-time stepper thread
                        if(step->doexit){
                                // Set the I/O pins to 0 to ensure that
                                // the stepper do not draw power when
                                // being idle.
                                setiopin(step,0,0);
                                setiopin(step,1,0);
                                setiopin(step,2,0);
                                setiopin(step,3,0);
                                pthread_exit(0);
                        }else if(step->getrpm){
                                step->getrpm = 0;
                                step->num_steps = num_steps;
                        }

                        // Acknowledge command:
                        sem_post(&step->rt_sem);
                }
                // Increase or decrease our stepping speed
                if(target_delay < delay){
                        delay-= 1000;
                }else if(target_delay > delay){
                        delay += 1000;
                }


                sleep_until(&ts,delay);

                setiopin(step,3,0);
                num_steps++;
                sleep_until(&ts,delay);

                setiopin(step,1,1);
                num_steps++;
                sleep_until(&ts,delay);

                setiopin(step,0,0);
                num_steps++;
                sleep_until(&ts,delay);

                setiopin(step,2,1);
                num_steps++;
                sleep_until(&ts,delay);

                setiopin(step,1,0);
                num_steps++;
                sleep_until(&ts,delay);

                setiopin(step,3,1);
                num_steps++;
                sleep_until(&ts,delay);

                setiopin(step,2,0);
                num_steps++;
                sleep_until(&ts,delay);

        }
        return 0;
}
