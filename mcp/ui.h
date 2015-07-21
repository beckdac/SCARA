#ifndef __UI_H__
#define __UI_H__

struct ui {
	sem_t sem;
	sem_t semRT;
};

void userInterfaceInit(void);
void userInterfaceThread(void *arg);

#endif /* __UI_H__ */
