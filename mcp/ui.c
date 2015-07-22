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
#include "stepper.h"
#include "util.h"
#include "laser.h"
#include "limits.h"
#include "core.h"
#include "ui.h"
#include "kinematics.h"

extern struct stepper step[2];
extern struct limits limits;
extern struct core core;

struct ui ui;

void userInterfaceInit(void) {
	sem_init(&ui.sem, 0, 0);
	sem_init(&ui.semRT, 0, 0);
}

void userInterfaceThread(void *arg) {
	// let the core know we are ready
	sem_post(&ui.semRT);

	// wait for go to display prompt
	sem_wait(&ui.sem);

        while(1) {
                char buf[80];
                printf("command (q, s, h, d, o, c, l, m, i): ");
                fgets(buf,79,stdin);
                if (buf[0] == 'q') {
			core.command = CORE_EXIT;
			sem_post(&core.sem);
			printf("posted core exit\n");
			// don't wait for ack, just exit
                        pthread_exit(0);
                } else if (buf[0] == 's') {
			core.command = CORE_STATUS;
			sem_post(&core.sem);
			sem_wait(&core.semRT);
			printf("Laser on pin %d is %s\n", laserGetPin(), (laserGetState() ? "on" : "off"));
			printf("Stepper 0\n\thomed\tmin %d\tmax %d\n\tlimit\tmin %d\tmax %d\n\tcenter\t%d\n\tpulseLen\t%d\n\tpulseLenTarget\t%d\n\tstepCurrent\t%d\n\tstepTarget\t%d\n",
				step[0].homed[0], step[0].homed[1], step[0].limit[0], step[0].limit[1], step[0].center,
				step[0].pulseLen, step[0].pulseLenTarget, step[0].stepCurrent, step[0].stepTarget
			);
			printf("Stepper 1\n\thomed\tmin %d\tmax %d\n\tlimit\tmin %d\tmax %d\n\tcenter\t%d\n\tpulseLen\t%d\n\tpulseLenTarget\t%d\n\tstepCurrent\t%d\n\tstepTarget\t%d\n",
				step[1].homed[0], step[1].homed[1], step[1].limit[0], step[1].limit[1], step[1].center,
				step[1].pulseLen, step[1].pulseLenTarget, step[1].stepCurrent, step[1].stepTarget
			);
			for (int i = 0; i < LIMIT_SWITCHES; ++i) {
				printf("Limit switch %d\t%d\n", i, limits.limit[i].state);
			}
		} else if (buf[0] == 'o') {
			core.command = CORE_HOME;
			sem_post(&core.sem);
                        sem_wait(&core.semRT);
		} else if (buf[0] == 'c') {
			core.command = CORE_CENTER;
			sem_post(&core.sem);
                        sem_wait(&core.semRT);
                } else if (buf[0] == 'h') {
			core.command = CORE_STOP;
			sem_post(&core.sem);
                        sem_wait(&core.semRT);
                } else if (buf[0] == 'd') {
			core.command = CORE_PWR_DN;
			sem_post(&core.sem);
                        sem_wait(&core.semRT);
		} else if (buf[0] == 'l') {
			core.command = CORE_LASER;
			if (core.laser)
				core.laser = 0;
			else
				core.laser = 1;
			sem_post(&core.sem);
                        sem_wait(&core.semRT);
                } else if (buf[0] == 'm') {
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
		} else if (buf[0] == 'i') {
			float x1, y1, x2, y2, S, E;
			int segments, stepsS, stepsE;
                        if (sscanf(buf,"i %f %f %f %f %d", &x1, &y1, &x2, &y2, &segments) == 4) {
				printf("rendering line from (%.2f, %.2f) to (%.2f, %.2f) in %d segments\n", x1, y1, x2, y2, segments);
				kinematicsInverse(x1, y1, L1_MM, L2_MM, &S, &E);
				stepsS = kinematicsRadToStep(S);
				stepsE = kinematicsRadToStep(E);
			}
		} else {
			warning("unknown keyboard command in userInterfaceThread: %s\n", buf);
		}
	}
}
