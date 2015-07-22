#ifndef __STEPPER_H__
#define __STEPPER_H__

// 28BYJ-48
#define STEPS_PER_REV 64. * (25792./405.)

#define DEFAULT_SLEEP 1000000

typedef enum stepperCommand { STEPPER_EXIT, STEPPER_STATUS, STEPPER_STOP, STEPPER_PWR_DN, STEPPER_MOVE_TO, STEPPER_HOME_MIN, STEPPER_HOME_MAX, STEPPER_CENTER, STEPPER_UNHOME } sCmd;

struct stepper {
        sem_t sem;
        sem_t semRT;

	uint8_t pins[4];

	sCmd command;

	unsigned int pulseLen;
	unsigned int pulseLenTarget;
	int stepCurrent;
	int stepTarget;

	uint8_t homed[2];	/* min & max, defaults to 0 - not homed */
	unsigned int limit[2];	/* min = 0, max = 1, values in steps of limits, min should always be 0, only valid once homed */
	unsigned int center;	
};

void stepperInit(struct stepper *step, int pin1, int pin2, int pin3, int pin4);
void *stepperThread(void *arg);

	// pin  state
#define STEPPER_SEQUENCE_N	8
#define STEPPER_SEQUENCE { \
	{ 1, 0, 0, 0 }, \
	{ 1, 1, 0, 0 }, \
	{ 0, 1, 0, 0 }, \
	{ 0, 1, 1, 0 }, \
	{ 0, 0, 1, 0 }, \
	{ 0, 0, 1, 1 }, \
	{ 0, 0, 0, 1 }, \
	{ 1, 0, 0, 1 } \
}

#endif /* __STEPPER_H__ */
