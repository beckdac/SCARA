#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <math.h>

#include "error.h"
#include "file.h"
#include "kinematics.h"
#include "task.h"

extern const tasks_table_t tasks_kinematics[];

int task_kinematics(int argc, char *argv[]) {
	int (*function)(int argc, char *argv[]);

	if (argc < 3) {
		int i;
TASK_GPIO_USAGE:
		warning("usage: %s %s <task> [<task parameters>]\n", argv[0], argv[1]);
       		warning("supported tasks are:\n", argv[0]);
       		for (i = 0; tasks_kinematics[i].name != NULL; ++i)
	       		warning("\t%s\n", tasks_kinematics[i].name);
		return EXIT_FAILURE;
	}

	function = task_lookup(tasks_kinematics, 2, argc, argv);

	if (function) {
		return function(argc, argv);
	}

	goto TASK_GPIO_USAGE;

	return EXIT_FAILURE;
}

int task_kinematics_ik(int argc, char *argv[]) {
	float x, y, L1, L2, S, E;

	if (argc != 5) {
KINEMATICS_TEST_USAGE:
		warning("usage: %s %s kinematics ik <X> <Y>\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	x = atof(argv[2]);
	y = atof(argv[3]);
	L1 = L1_MM;
	L2 = L1_MM;
	printf("x = %.2f   y = %.2f   L1 = %.2f   L2 = %.2f\n", x, y, L1, L2);
	kinematicsInverse(x, y, L1, L2, &S, &E);
	printf("raw  -  S = %.4f   E = %.4f\n", S, E);
	printf("deg  -  S = %.4f   E = %.4f\n", kinematicsRadToDeg(S), kinematicsRadToDeg(E));
	printf("rad  -  S = %.4f   E = %.4f\n", kinematicsDegToRad(kinematicsRadToDeg(S)), kinematicsDegToRad(kinematicsRadToDeg(E)));
	printf("step -  S = %4d   E = %4d\n", kinematicsRadToStep(S), kinematicsRadToStep(E));
	kinematicsForward(&x, &y, L1, L2, S, E);
	printf("x = %.4f   y = %.4f\n", x, y);

	return EXIT_SUCCESS;
}

int task_kinematics_line(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}

int task_kinematics_arc(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}
