#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <math.h>

#include "error.h"
#include "file.h"
#include "gpio.h"
#include "task.h"
#include "compatability.h"
#include "stepper.h"
#include "util.h"
#include "laser.h"
#include "limits.h"

extern struct stepper step[2];

extern struct limits limits;

void userInterfaceThread(void *arg) {
        while(1) {
                char buf[80];
                printf("Enter command (q, s, h, d, l, m): ");
                fgets(buf,79,stdin);
                if (buf[0] == 'q') {
			step[0].command = STEPPER_EXIT;
			step[1].command = STEPPER_EXIT;
                        sem_post(&step[0].sem);
                        sem_post(&step[1].sem);
			limits.command = LIMIT_EXIT;
			sem_post(&limits.sem);
			// don't wait for ack, just exit
                        pthread_exit(0);
                } else if (buf[0] == 's'){
			printf("Laser on pin %d is %s\n", laserGetPin(), (laserGetState() ? "on" : "off"));
                        step[0].command = STEPPER_STATUS;
                        step[1].command = STEPPER_STATUS;
                        sem_post(&step[0].sem);
                        sem_post(&step[1].sem);
                        sem_wait(&step[0].semRT);
                        sem_wait(&step[1].semRT);
			limits.command = LIMIT_STATUS;
			sem_post(&limits.sem);
			sem_wait(&limits.semRT);
			printf("Stepper 0\n\tpulseLen\t%d\n\tpulseLenTarget\t%d\n\tstepCurrent\t%d\n\tstepTarget\t%d\n",
				step[0].pulseLen, step[0].pulseLenTarget, step[0].stepCurrent, step[0].stepTarget);
			printf("Stepper 1\n\tpulseLen\t%d\n\tpulseLenTarget\t%d\n\tstepCurrent\t%d\n\tstepTarget\t%d\n",
				step[1].pulseLen, step[1].pulseLenTarget, step[1].stepCurrent, step[1].stepTarget);
			for (int i = 0; i < LIMIT_SWITCHES; ++i) {
				printf("Limit switch %d\t%d\n", i, limits.limit[i].state);
			}
                } else if (buf[0] == 'h'){
                        step[0].command = STEPPER_STOP;
                        step[1].command = STEPPER_STOP;
                        sem_post(&step[0].sem);
                        sem_post(&step[1].sem);
                        sem_wait(&step[0].semRT);
                        sem_wait(&step[1].semRT);
                } else if (buf[0] == 'd'){
			laserOff();
                        step[0].command = STEPPER_PWR_DN;
                        step[1].command = STEPPER_PWR_DN;
                        sem_post(&step[0].sem);
                        sem_post(&step[1].sem);
                        sem_wait(&step[0].semRT);
                        sem_wait(&step[1].semRT);
		} else if (buf[0] == 'l') {
			laserToggle();
                } else if (buf[0] == 'm'){
			unsigned stepno;
                        int stepTarget;
                        unsigned int pulseLenTarget;
                        if (sscanf(buf,"m %d %d %d", &stepno, &stepTarget, &pulseLenTarget) == 3) {
				printf("MOVE_TO given to stepper %d w/ target step %d and pulse len of %d\n", stepno, stepTarget, pulseLenTarget);
                        	step[stepno].command = STEPPER_MOVE_TO;
				step[stepno].pulseLenTarget = pulseLenTarget;
                                step[stepno].stepTarget = stepTarget;
                                sem_post(&step[stepno].sem);
                                sem_wait(&step[stepno].semRT);
                        }
		} else {
			warning("unknown keyboard command in userInterfaceThread: %s\n", buf);
		}
	}
}
