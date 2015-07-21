#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "error.h"
#include "file.h"
#include "gpio.h"
#include "task.h"
#include "compatability.h"
#include "stepper.h"
#include "ui.h"
#include "util.h"

struct stepper step[2];

uint8_t stepSequence[STEPPER_SEQUENCE_N][4] = STEPPER_SEQUENCE;

void stepperInit(struct stepper *step, int pin1, int pin2, int pin3, int pin4) {
	step->pins[0] = pin1;
	step->pins[1] = pin2;
	step->pins[2] = pin3;
	step->pins[3] = pin4;

        gpio_export(pin1);
        gpio_direction(pin1, GPIO_DIR_OUT);
        gpio_export(pin2);
        gpio_direction(pin2, GPIO_DIR_OUT);
        gpio_export(pin3);
        gpio_direction(pin3, GPIO_DIR_OUT);
        gpio_export(pin4);
        gpio_direction(pin4, GPIO_DIR_OUT);

        sem_init(&step->sem,0,0);
        sem_init(&step->semRT,0,0);

	step->pulseLen = 0;
	step->pulseLenTarget = 0;
	step->stepCurrent = 0;
	step->stepTarget = 0;
}

void stepperPowerDown(struct stepper *step) {
	gpio_write(step->pins[0], 0);
	gpio_write(step->pins[1], 0);
	gpio_write(step->pins[2], 0);
	gpio_write(step->pins[3], 0);
}

void *stepperThread(void *arg) {
	struct stepper *step = (struct stepper *)arg;
        struct timespec ts;
        struct sched_param sp;
	// local variables
	sCmd command = STEPPER_PWR_DN;
	unsigned int pulseLen = 0, pulseLenTarget = 0;
	int stepCurrent = 0, stepTarget = 0;
	int seqIndex = 0;

	// enable real time priority for this thread
        sp.sched_priority = 30;
        if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)){
		warning("unable to set stepper thread to realtime priority\n");
        }

	// initialize monotonically increasing clock
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_nsec = 0;

	// start in the powered down state
	stepperPowerDown(step);
        while(1) {
                if (!sem_trywait(&step->sem)) {
			printf("stepper command = %d\n", step->command);
			command = step->command;
			switch (command) {
				case STEPPER_EXIT:
					stepperPowerDown(step);
					pthread_exit(0);
					break;
				case STEPPER_STATUS:
					step->stepCurrent = stepCurrent;
					step->stepTarget = stepTarget;
					step->pulseLen = pulseLen;
					step->pulseLenTarget = pulseLenTarget;
					break;
				case STEPPER_STOP:
					stepTarget = stepCurrent;
					pulseLen = 0;
					break;
				case STEPPER_PWR_DN:
					stepperPowerDown(step);
					break;
				case STEPPER_MOVE_TO:
					pulseLenTarget = step->pulseLenTarget;
					stepTarget = step->stepTarget;
					break;
				default:
					fatal_error("unexpected command for stepper thread\n");
					break;
			};

			// ack
                        sem_post(&step->semRT);
		}
		if (command == STEPPER_MOVE_TO) {
			// accelerate or deccelerate
			if (pulseLen == 0)
				pulseLen = pulseLenTarget;
			if (pulseLenTarget < pulseLen) {
				pulseLen -= 1000;
				if (pulseLen < pulseLenTarget)
					pulseLen = pulseLenTarget;
			} else if (pulseLenTarget > pulseLen) {
				pulseLen += 1000;
				if (pulseLen > pulseLenTarget)
					pulseLen = pulseLenTarget;
			}
			if (stepTarget < stepCurrent) {
				seqIndex--;
				if (seqIndex < 0)
					seqIndex = STEPPER_SEQUENCE_N - 1;
				stepCurrent--;
			} else if (stepTarget > stepCurrent) {
				seqIndex++;
				if (seqIndex > STEPPER_SEQUENCE_N - 1)
					seqIndex = 0;
				stepCurrent++;
			} else {
				command = STEPPER_STOP;
#warning "this probably shouldn't be here"
				stepperPowerDown(step);
			}
			if (command == STEPPER_MOVE_TO) {
				for (int i = 0; i < 4; ++i) {
					gpio_write(step->pins[i], stepSequence[seqIndex][i]);
				}
			}
			sleep_until(&ts, pulseLen);
		} else {
			sleep_until(&ts, DEFAULT_SLEEP);
		}

#if 0
                setiopin(step,0,1);
                num_steps++;
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
#endif

        }
        return 0;
}
