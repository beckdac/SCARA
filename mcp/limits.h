#ifndef __LIMITS_H__
#define __LIMITS_H__

#define LIMIT_SWITCHES	4	// will be 6 when the Z-axis gets implemented

// GPIO pin #s
#define LIMITS_SHLDR_MIN_PIN	16
#define LIMITS_SHLDR_MAX_PIN	20
#define LIMITS_FOREARM_MIN_PIN	21
#define LIMITS_FOREARM_MAX_PIN	26
// structure indexes
#define LIMITS_SHLDR_MIN	0
#define LIMITS_SHLDR_MAX	1
#define LIMITS_FOREARM_MIN	2
#define LIMITS_FOREARM_MAX	3

typedef enum limitCommand { LIMIT_EXIT, LIMIT_STATUS } limitCmd;

struct limit {
	uint8_t pin;

	unsigned int state;
};

struct limits {
        sem_t sem;
        sem_t semRT;

	limitCmd command;

	struct limit limit[LIMIT_SWITCHES];
};

void limitsInit(void);
void *limitsThread(void *arg);

#endif /* __LIMITS_H__ */
