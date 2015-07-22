#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <math.h>

#include "error.h"
#include "file.h"
#include "gpio.h"
#include "task.h"
#include "queue.h"
#include "stepper.h"
#include "ui.h"
#include "util.h"
#include "laser.h"
#include "limits.h"
#include "core.h"

extern const tasks_table_t tasks_core[];
extern struct stepper step[2];

int task_core(int argc, char *argv[]) {
	int (*function)(int argc, char *argv[]);

	if (argc < 3) {
		int i;
TASK_CORE_USAGE:
		warning("usage: %s %s <task> [<task parameters>]\n", argv[0], argv[1]);
       		warning("supported tasks are:\n", argv[0]);
       		for (i = 0; tasks_core[i].name != NULL; ++i)
	       		warning("\t%s\n", tasks_core[i].name);
		return EXIT_FAILURE;
	}

	function = task_lookup(tasks_core, 2, argc, argv);

	if (function) {
		struct timeval  tv1, tv2;
		int rv;
		double time_diff;

		gettimeofday(&tv1, NULL);

		rv = function(argc, argv);

		gettimeofday(&tv2, NULL);
		time_diff = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000. + (double) (tv2.tv_sec - tv1.tv_sec);
		printf("session lasted %.4f seconds\n", time_diff);

		return rv;
	}

	goto TASK_CORE_USAGE;

	return EXIT_FAILURE;
}

int task_core_run(int argc, char *argv[]) {
	if (argc != 3) {
		warning("usage: %s %s %s\n", argv[0], argv[1], argv[2]);
		return EXIT_FAILURE;
	}

	coreRun();

	return EXIT_SUCCESS;
}
