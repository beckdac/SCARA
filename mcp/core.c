#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>

#include "error.h"
#include "file.h"
#include "gpio.h"
#include "task.h"
#include "stepper.h"
#include "ui.h"
#include "util.h"
#include "limits.h"
#include "core.h"
#include "laser.h"

#undef LOCK_MEM
#define LOCK_MEM

#undef ENABLE_STEPPERS
#define ENABLE_STEPPERS

#ifdef ENABLE_STEPPERS
extern struct stepper step[2];
#endif

#undef ENABLE_LIMITS
#define ENABLE_LIMITS

#ifdef ENABLE_LIMITS
extern struct limits limits;
#endif

extern struct ui ui;
struct core core;

static void coreInitSemaphores(void) {
	//printf("core initializing semaphores\n");
	sem_init(&core.sem, 0, 0);
	sem_init(&core.semRT, 0, 0);
}

static void coreInitMemory(void) {
#ifdef LOCK_MEM
	// lock all memory to prevent swapping
	if (mlockall(MCL_FUTURE|MCL_CURRENT)) {
		warning("unable to lock memory with mlockall()\n");
	}
#endif
}

static void coreInitGPIO(void) {
	gpio_init();
}

static void coreInitLaser(void) {
	laserInit(LASER_DEFAULT_PIN);
}

static void coreInitLimits(void) {
#ifdef ENABLE_LIMITS
	limitsInit();
#endif
}

static void coreInitSteppers(void) {
#ifdef ENABLE_STEPPERS
	//printf("initializing steppers\n");
	stepperInit(&step[0],  4, 17, 18, 27);
	stepperInit(&step[1], 22, 23, 24, 25);
	//printf("done\n");
#endif
}

static void coreInitUserInterface(void) {
	userInterfaceInit();
}

static void coreInit(void) {
	memset(&core, 0, sizeof(struct core));
	coreInitMemory();
	coreInitGPIO();
	coreInitLaser();
	coreInitLimits();
	coreInitSteppers();
	coreInitUserInterface();
	coreInitSemaphores();
}

/////////////

static void coreCleanupLaser(void) {
	laserCleanup();
}

static void coreCleanupGPIO(void) {
	gpio_done();
}

static void coreCleanup(void) {
	coreCleanupLaser();
	coreCleanupGPIO();
}

/////////////

static void coreExit(void) {
#ifdef ENABLE_STEPPERS
	step[0].command = STEPPER_EXIT;
	sem_post(&step[0].sem);
	step[1].command = STEPPER_EXIT;
	sem_post(&step[1].sem);
#endif
#ifdef ENABLE_LIMITS
	limits.command = LIMIT_EXIT;
	sem_post(&limits.sem);
#endif
	// don't wait for ack, just exit
}

static void coreStatus(void) {
#ifdef ENABLE_STEPPERS
	step[0].command = STEPPER_STATUS;
	sem_post(&step[0].sem);
	step[1].command = STEPPER_STATUS;
	sem_post(&step[1].sem);
#endif
#ifdef ENABLE_LIMITS
	limits.command = LIMIT_STATUS;
	sem_post(&limits.sem);
#endif

#ifdef ENABLE_STEPPERS
	sem_wait(&step[0].semRT);
	sem_wait(&step[1].semRT);
#endif
#ifdef ENABLE_LIMITS
	sem_wait(&limits.semRT);
#endif
}

static void corePowerDown(void) {
	laserOff();
	core.laser = 0;

#ifdef ENABLE_STEPPERS
	step[0].command = STEPPER_PWR_DN;
	sem_post(&step[0].sem);
	step[1].command = STEPPER_PWR_DN;
	sem_post(&step[1].sem);

	sem_wait(&step[0].semRT);
	sem_wait(&step[1].semRT);
#endif
}

static void coreStop(void) {
#ifdef ENABLE_STEPPERS
	step[0].command = STEPPER_STOP;
	sem_post(&step[0].sem);
	step[1].command = STEPPER_STOP;
	sem_post(&step[1].sem);

	sem_wait(&step[0].semRT);
	sem_wait(&step[1].semRT);
#endif
}

void *coreThread(void *arg) {
	struct timespec ts;
	struct sched_param sp;
	// local variables
	coreCmd command = CORE_PWR_DN, state = CORE_PWR_DN;
	uint8_t homed = 0, laser = 0;

	printf("coreThread started...\n");

	// enable real time priority for this thread
	sp.sched_priority = 30;
	if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp)){
		warning("unable to set core thread to realtime priority\n");
	}

	// initialize monotonically increasing clock
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_nsec = 0;

	// wait for all threads to post
	printf("coreThread waiting for threads to post...\n");
#ifdef ENABLE_STEPPERS
	sem_wait(&step[0].semRT);
	sem_wait(&step[1].semRT);
#endif
#ifdef ENABLE_LIMITS
	sem_wait(&limits.semRT);
#endif
	sem_wait(&ui.semRT);

	// begin in powered down state
	corePowerDown();
	printf("coreThread entering main loop...\n");
	sem_post(&ui.sem);
	while (1) {
		if (!sem_trywait(&core.sem)) {
			printf("core command = %d\n", core.command);
			command = core.command;
			switch (command) {
				case CORE_EXIT:
					coreExit();
					pthread_exit(0);
					break;
				case CORE_STATUS:
					coreStatus();
					break;
				case CORE_PWR_DN:
					state = command;
					corePowerDown();
					break;
				case CORE_STOP:
					state = command;
					coreStop();
					break;
				case CORE_HOME:
					state = command;
					step[0].command = STEPPER_MOVE_TO;
					step[0].stepTarget = -STEPS_PER_REV;
					step[0].pulseLenTarget = DEFAULT_SLEEP;
					sem_post(&step[0].sem);
					//step[1].command = STEPPER_MOVE_TO;
					//step[1].stepTarget = -STEPS_PER_REV;
					//step[1].pulseLenTarget = DEFAULT_SLEEP;
					//sem_post(&step[1].sem);
					sem_wait(&step[0].semRT);
					//sem_wait(&step[1].semRT);
					break;
				case CORE_LASER:
					laser = core.laser;
					if (laser)
						laserOn();
					else
						laserOff();
					break;
				case CORE_LIMIT:
					{
					printf("core_limit\n");
					uint8_t wait_s0 = 0, wait_s1 = 0;
					corePowerDown();
					if (!limits.limit[0].state) {
						printf("arm min limit triggered\n");
						if (state == CORE_HOME) {
							step[0].command = STEPPER_HOME_MIN;
							sem_post(&step[0].sem);
							wait_s0 = 1;
						} else {
							state = command = CORE_PWR_DN;
							corePowerDown();
						}
					}
					if (!limits.limit[1].state) {
						printf("arm max limit triggered\n");
						if (state == CORE_HOME) {
							step[0].command = STEPPER_HOME_MAX;
							sem_post(&step[0].sem);
							wait_s0 = 1;
						} else {
							state = command = CORE_PWR_DN;
							corePowerDown();
						}
					}
					if (!limits.limit[2].state) {
						printf("forearm min limit triggered\n");
						if (state == CORE_HOME) {
							step[1].command = STEPPER_HOME_MIN;
							sem_post(&step[0].sem);
							wait_s1 = 1;
						} else {
							state = command = CORE_PWR_DN;
							corePowerDown();
						}
					}
					if (!limits.limit[3].state) {
						printf("forearm min limit triggered\n");
						if (state == CORE_HOME) {
							step[1].command = STEPPER_HOME_MAX;
							sem_post(&step[0].sem);
							wait_s1 = 1;
						} else {
							state = command = CORE_PWR_DN;
							corePowerDown();
						}
					}
					if (wait_s0) {
						sem_wait(&step[0].semRT);
					}
					if (wait_s1) {
						sem_wait(&step[1].semRT);
					}
					}
					break;
				default:
					break;
			};
			// ack
			sem_post(&core.semRT);
		}
		sleep_until(&ts, DEFAULT_SLEEP);
	}

	return 0;
}

/////////////

void coreRun(void) {
	pthread_t stepperThreads[2];
	pthread_t uiThread, limThread;
	pthread_t coreExecThread;

	coreInit();

	// spawn threads
	pthread_create(&uiThread, NULL, (void *)(userInterfaceThread), NULL);
#ifdef ENABLE_STEPPERS
	pthread_create(&stepperThreads[0], NULL, (void *)(stepperThread), (void *)&step[0]);
	pthread_create(&stepperThreads[1], NULL, (void *)(stepperThread), (void *)&step[1]);
#endif
#ifdef ENABLE_LIMITS
	pthread_create(&limThread, NULL, (void *)(limitsThread), NULL);
#endif
	pthread_create(&coreExecThread, NULL, (void *)(coreThread), NULL);

	// suspend until joined with the other threads
	pthread_join(uiThread, NULL);
	printf("collected user interface thread\n");
#ifdef ENABLE_STEPPERS
	pthread_join(stepperThreads[0], NULL);
	printf("collected stepper 0 thread\n");
	pthread_join(stepperThreads[1], NULL);
	printf("collected stepper 1 thread\n");
#endif
#ifdef ENABLE_LIMITS
	pthread_join(limThread, NULL);
	printf("collected limit thread\n");
#endif
	pthread_join(coreExecThread, NULL);
	printf("collected core thread\n");
	
	coreCleanup();
}
