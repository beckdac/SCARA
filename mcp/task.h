#ifndef _TASK_H_
#define _TASK_H_

typedef struct tasks_table {
	char *name;
        int (*function)(int argc, char *argv[]);
} tasks_table_t;

int task_parse(int level, int argc, char *argv[]);
int (*task_lookup(const tasks_table_t *lookup_table, int argv_index, int argc, char *argv[]))(int argc, char *argv[]);

// task_gpio.c
int task_gpio(int argc, char *argv[]);
int task_gpio_test_speed(int argc, char *argv[]);
int task_gpio_test_poll(int argc, char *argv[]);

// task_core.c
int task_core(int argc, char *argv[]);
int task_calibrate(int argc, char *argv[]);

#endif
