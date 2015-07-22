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
#include "queue.h"
#include "stepper.h"
#include "ui.h"
#include "util.h"
#include "core.h"

struct stepper step[STEPPER_COUNT];
extern struct core core;

uint8_t stepSequence[STEPPER_SEQUENCE_N][4] = STEPPER_SEQUENCE;

#define SPEED_RAMP
#undef SPEED_RAMP

void stepperInit(uint8_t index, int pin1, int pin2, int pin3, int pin4) {
	if (index >= STEPPER_COUNT) {
		fatal_error("stepper index (%d) is out of range\n", index);
	}
	struct stepper *s = &step[index];
	//printf("stepperInit @ 0x%x\n", step);

	s->pins[0] = pin1;
	s->pins[1] = pin2;
	s->pins[2] = pin3;
	s->pins[3] = pin4;

        gpio_export(pin1);
        gpio_direction(pin1, GPIO_DIR_OUT);
        gpio_export(pin2);
        gpio_direction(pin2, GPIO_DIR_OUT);
        gpio_export(pin3);
        gpio_direction(pin3, GPIO_DIR_OUT);
        gpio_export(pin4);
        gpio_direction(pin4, GPIO_DIR_OUT);

	//printf("initializing semaphore for stepper on pins %d - %d - %d - %d\n", pin1, pin2, pin3, pin4);
        sem_init(&s->sem, 0, 0);
        sem_init(&s->semRT, 0, 0);

	s->index = index;

	s->pulseLen = 0;
	s->pulseLenTarget = 0;
	s->stepCurrent = 0;
	s->stepTarget = 0;

	s->homed[0] = 0;
	s->homed[1] = 0;
	s->limit[0] = 0;
	s->limit[1] = 0;
}

static void stepperCleanup(struct stepper *step) {
	gpio_unexport(step->pins[0]);
	gpio_unexport(step->pins[1]);
	gpio_unexport(step->pins[2]);
	gpio_unexport(step->pins[3]);
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
	uint8_t homed[2] = { 0, 0 };
	unsigned int limit[2] = { 0, 0 }, center = 0;
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
	printf("stepper alive: %d - %d - %d - %d\n", step->pins[0], step->pins[1], step->pins[2], step->pins[3]);
	stepperPowerDown(step);

	// notify core we are ready
	sem_post(&step->semRT);
	
        while(1) {
		int sc;
		sem_getvalue(&step->sem, &sc);
		if (sc)
			printf("sc=%d\n", sc);
                if (!sem_trywait(&step->sem)) {
			//printf("stepper command = %d\n", step->command);
			command = step->command;
			switch (command) {
				case STEPPER_EXIT:
					stepperPowerDown(step);
					stepperCleanup(step);
					pthread_exit(0);
					break;
				case STEPPER_STATUS:
					step->stepCurrent = stepCurrent;
					step->stepTarget = stepTarget;
					step->pulseLen = pulseLen;
					step->pulseLenTarget = pulseLenTarget;
					step->homed[0] = homed[0];
					step->homed[1] = homed[1];
					step->limit[0] = limit[0];
					step->limit[1] = limit[1];
					step->center = center;
					// ack
                        		sem_post(&step->semRT);
					break;
				case STEPPER_STOP:
					stepTarget = stepCurrent;
					pulseLen = 0;
                        		sem_post(&step->semRT);
					break;
				case STEPPER_PWR_DN:
					stepperPowerDown(step);
                        		sem_post(&step->semRT);
					break;
				case STEPPER_CENTER:
					if (homed[0] && homed[1]) {
						pulseLenTarget = step->pulseLenTarget;
						stepTarget = center;
						command = STEPPER_MOVE_TO;
						coreIncrementMovesInProgress(step->index);
					} else {
						warning("cannot center an unhomed axis!\n");
					}
                        		sem_post(&step->semRT);
					break;
				case STEPPER_MOVE_TO:
					pulseLenTarget = step->pulseLenTarget;
					printf("move_to\t%d\t%d\t%d\n", step->index, step->stepTarget, step->pulseLenTarget);
					if (homed[0] && homed[1]) {
						if (step->stepTarget >= limit[0] && step->stepTarget <= limit[1])
							stepTarget = step->stepTarget;
						else {
							if (step->stepTarget < limit[0]) {
								stepTarget = limit[0];
								warning("request for position less than homed range (%d)\n", step->stepTarget);
							} else {
								stepTarget = limit[1];
								warning("request for position larger than homed range (%d)\n", step->stepTarget);
							}
						}
					} else {
						stepTarget = step->stepTarget;
						warning("moving on unhomed axis to %d\n", stepTarget);
					}
					coreIncrementMovesInProgress(step->index);
                        		sem_post(&step->semRT);
					break;
				case STEPPER_UNHOME:
					homed[0] = 0;
					homed[1] = 0;
					limit[0] = 0;
					limit[1] = 0;
                        		sem_post(&step->semRT);
					break;
				case STEPPER_HOME_MIN:
					stepperPowerDown(step);
					coreDecrementMovesInProgress(step->index);
					homed[0] = 1;
					limit[0] = 0;
					stepCurrent = 0;
					stepTarget = 0;
					command = STEPPER_PWR_DN;
					printf("stepper homed min (pins %d - %d - %d - %d)\n", step->pins[0], step->pins[1], step->pins[2], step->pins[3]);
                        		sem_post(&step->semRT);
					break;
				case STEPPER_HOME_MAX:
					stepperPowerDown(step);
					coreDecrementMovesInProgress(step->index);
					if (homed[0]) {
						homed[1] = 1;
						limit[1] = stepCurrent;
						center = (limit[1] - limit[0]) / 2;
						step->center = center;
						stepTarget = 0;
						printf("stepper homed max at %d steps (pins %d - %d - %d - %d)\n", stepCurrent, step->pins[0], step->pins[1], step->pins[2], step->pins[3]);
					} else {
						warning("cannot home max before homing min!\n");
					}
					command = STEPPER_PWR_DN;
                        		sem_post(&step->semRT);
					break;
				default:
					fatal_error("unexpected command for stepper thread\n");
					break;
			};
		}
		if (command == STEPPER_MOVE_TO) {
#ifdef SPEED_RAMP
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
#else
			pulseLen = pulseLenTarget;
#endif
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
				printf("stepper %d reached target step %d\n", step->index, stepTarget);
#warning "do we really want to power down?"
#warning "this only works because core is the only thread updating this?"
				command = STEPPER_PWR_DN;
				stepperPowerDown(step);
				coreDecrementMovesInProgress(step->index);
				core.command = CORE_MOVE_TO_COMPLETE;
				sem_post(&core.sem);
				sem_wait(&core.semRT);
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
        }
        return 0;
}
