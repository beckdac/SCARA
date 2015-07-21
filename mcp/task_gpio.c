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

extern const tasks_table_t tasks_gpio[];

int task_gpio(int argc, char *argv[]) {
	int (*function)(int argc, char *argv[]);

	if (argc < 3) {
		int i;
TASK_GPIO_USAGE:
		warning("usage: %s %s <task> [<task parameters>]\n", argv[0], argv[1]);
       		warning("supported tasks are:\n", argv[0]);
       		for (i = 0; tasks_gpio[i].name != NULL; ++i)
	       		warning("\t%s\n", tasks_gpio[i].name);
		return EXIT_FAILURE;
	}

	function = task_lookup(tasks_gpio, 2, argc, argv);

	if (function) {
		return function(argc, argv);
	}

	goto TASK_GPIO_USAGE;

	return EXIT_FAILURE;
}

int task_gpio_test_speed(int argc, char *argv[]) {
	int pin, cycles, i;
	struct timeval  tv1, tv2;
	double time_diff;

	if (argc != 5) {
		warning("usage: %s %s %s <GPIO pin> <bit flip cycles>\n", argv[0], argv[1], argv[2]);
		return EXIT_FAILURE;
	}
	pin = atoi(argv[3]);
	cycles = atoi(argv[4]);

	printf("%s: bit flip GPIO pin %d for %d cycles\n", argv[0], pin, cycles);

	gpio_init();

	gpio_export(pin);
	gpio_direction(pin, GPIO_DIR_OUT);

	gettimeofday(&tv1, NULL);
	// flip the pin as quickly as possible
	for (i = 0; i < cycles; ++i) {
		gpio_write(pin, i % 2);
	}
	gpio_write(pin, 0);
	gettimeofday(&tv2, NULL);
	time_diff = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000. + (double) (tv2.tv_sec - tv1.tv_sec);
	printf("wall clock of %.4f seconds for %g pin changes per second\n", time_diff, (double)cycles/time_diff);

	gpio_unexport(pin);

	gpio_done();
	
	return EXIT_SUCCESS;
}

int task_gpio_test_poll(int argc, char *argv[]) {
	int pin, timeout;

	if (argc != 5) {
		warning("usage: %s %s %s <GPIO pin> <poll timeout in milliseconds>\n", argv[0], argv[1], argv[2]);
		return EXIT_FAILURE;
	}
	pin = atoi(argv[3]);
	timeout = atoi(argv[4]);

	gpio_init();

	gpio_export(pin);

while(1) {
	printf("%s: switching to input on pin %d and polling\n", argv[0], pin);
	gpio_direction(pin, GPIO_DIR_IN);
	printf("%s: pin %d = %d\n", argv[0], pin, gpio_read(pin));
	printf("%s: non-blocking poll\n", argv[0]);
	gpio_poll(pin, 0, GPIO_EDGE_BOTH);
	printf("%s: pin %d = %d\n", argv[0], pin, gpio_read(pin));
	printf("%s: polling for %d ms\n", argv[0], timeout);
	gpio_poll(pin, timeout, GPIO_EDGE_BOTH);
	printf("%s: pin %d = %d\n", argv[0], pin, gpio_read(pin));
}

	gpio_unexport(pin);

	gpio_done();
	
	return EXIT_SUCCESS;
}
