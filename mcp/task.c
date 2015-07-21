#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <math.h>

#include "error.h"
#include "file.h"
#include "gpio.h"
#include "task.h"
#include "compatability.h"

const tasks_table_t tasks[] = { \
	{ "gpio",	&task_gpio }, \
	{ "core",	&task_core }, \
	{ NULL,		NULL } /* end */
};

const tasks_table_t tasks_gpio[] = { \
	{ "test_speed",	&task_gpio_test_speed }, \
	{ "test_poll",	&task_gpio_test_poll }, \
	{ NULL,		NULL } /* end */
};

const tasks_table_t tasks_core[] = { \
	{ "calibrate",	&task_calibrate }, \
	{ NULL,		NULL } /* end */
};

int task_parse(int level, int argc, char *argv[]) {
	int i;
	int (*function)(int argc, char *argv[]);

	if (argc < level + 1) {
		goto TASK_PARSE_USAGE;
	}

	function = task_lookup(tasks, level, argc, argv);

	if (function) {
		return function(argc, argv);
	}

TASK_PARSE_USAGE:
	warning("usage: %s <task> [<task parameters>]\n", argv[0]);
       	warning("supported tasks are:\n", argv[0]);
       	for (i = 0; tasks[i].name != NULL; ++i)
	       	warning("\t%s\n", tasks[i].name);

	return EXIT_FAILURE;
}

int (*task_lookup(const tasks_table_t *lookup_table, int argv_index, int argc, char *argv[]))(int argc, char *argv[]) {
	int i;

	if (argv_index >= argc) {
		warning("%s: argv index %d is out of range (0 to %d)\n", __PRETTY_FUNCTION__, argv_index, argc - 1);
		return NULL;
	}

	for (i = 0; lookup_table[i].name != NULL; ++i) {
		if (strcmp(lookup_table[i].name, argv[argv_index]) == 0) {
			return lookup_table[i].function;
			break;
		}
	}
	warning("%s: unhandled token '%s'\n", __PRETTY_FUNCTION__, argv[argv_index]);
	return NULL;
}
