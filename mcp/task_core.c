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
#include "compatability.h"
#include "stepper.h"
#include "ui.h"
#include "util.h"
#include "laser.h"
#include "limits.h"

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
		int retval;

		// lock all memory to prevent swapping
		if (mlockall(MCL_FUTURE|MCL_CURRENT)) {
			warning("unable to lock memory with mlockall()\n");
		}

		// perform initialization
		gpio_init();
		stepperInit(&step[0],  4, 17, 18, 27);
		stepperInit(&step[1], 22, 23, 24, 25);

		// do the execution
		retval = function(argc, argv);

		// shut everything down
		for (int s = 0; s < 2; ++s)
			for (int p = 0; p < 4; ++p)
				gpio_unexport(step[s].pins[p]);
		gpio_done();

		// return result
		return retval;
	}

	goto TASK_CORE_USAGE;

	return EXIT_FAILURE;
}

int task_calibrate(int argc, char *argv[]) {
	struct timeval  tv1, tv2;
	double time_diff;

	pthread_t stepperThreads[2];
	pthread_t uiThread, limThread;

	if (argc != 3) {
		warning("usage: %s %s %s\n", argv[0], argv[1], argv[2]);
		return EXIT_FAILURE;
	}

	gettimeofday(&tv1, NULL);

	laserInit(LASER_DEFAULT_PIN);

	pthread_create(&uiThread, NULL, (void *)(userInterfaceThread), NULL);
        pthread_create(&stepperThreads[0], NULL, (void *)(stepperThread), (void *)&step[0]);
        pthread_create(&stepperThreads[1], NULL, (void *)(stepperThread), (void *)&step[1]);
        pthread_create(&limThread, NULL, (void *)(limitsThread), NULL);

	// join with the other threads
        pthread_join(uiThread, NULL);
	printf("collected user interface thread\n");
        pthread_join(stepperThreads[0], NULL);
	printf("collected stepper 0 thread\n");
        pthread_join(stepperThreads[1], NULL);
	printf("collected stepper 1 thread\n");
	pthread_join(limThread, NULL);
	printf("collected limit thread\n");

	laserCleanup();

	gettimeofday(&tv2, NULL);
	time_diff = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000. + (double) (tv2.tv_sec - tv1.tv_sec);
	printf("wall clock of %.4f seconds for calibration\n", time_diff);

	return EXIT_SUCCESS;
}
