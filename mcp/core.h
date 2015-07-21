#ifndef __CORE_H__
#define __CORE_H__

typedef enum coreCommand { CORE_EXIT, CORE_STATUS, CORE_PWR_DN, CORE_STOP, CORE_HOME, CORE_LASER } coreCmd;

void coreRun(void);

struct core {
	sem_t	sem;
	sem_t	semRT;

	coreCmd command;

	uint8_t homed;
	uint8_t laser;
};

#endif /* __CORE_H__ */
