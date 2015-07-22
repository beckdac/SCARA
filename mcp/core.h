#ifndef __CORE_H__
#define __CORE_H__

#define CORE_DEFAULT_QUEUE_LENGTH	1000

typedef enum coreCommand { CORE_EXIT, CORE_STATUS, CORE_STOP, CORE_PWR_DN, CORE_LIMIT, CORE_HOME, CORE_CENTER, CORE_LASER, CORE_MOVE_TO_COMPLETE, CORE_BATCH_FILE } coreCmd;

void coreRun(void);

struct core {
	sem_t	sem;
	sem_t	semRT;

	coreCmd command;

	uint8_t homed;
	uint8_t laser;

	queue queue;
};

#endif /* __CORE_H__ */
