#ifndef __CORE_H__
#define __CORE_H__

typedef enum coreCommand { CORE_EXIT, CORE_STATUS, CORE_STOP, CORE_PWR_DN, CORE_LIMIT, CORE_HOME, CORE_CENTER, CORE_LASER, CORE_MOVE_TO_COMPLETE, CORE_BATCH_FILE } coreCmd;

void coreRun(void);

struct core {
	sem_t	sem;
	sem_t	semRT;

	coreCmd command;

	uint8_t homed;
	uint8_t laser;
	file_t *batch_file;
};

#endif /* __CORE_H__ */
