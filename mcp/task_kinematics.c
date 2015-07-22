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
		warning("usage: %s %s kinematics ik <x> <y>\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	x = atof(argv[3]);
	y = atof(argv[4]);
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
	float x = 0, y = 0, x1, y1, x2, y2, S, E;
	unsigned int segments;
	float d2, d, segmentLen;
	int i;

	if (argc != 8) {
		warning("usage: %s %s kinematics line <x1> <y1> <x2> <y2> <segments>\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	x1 = atof(argv[3]);
	y1 = atof(argv[4]);
	x2 = atof(argv[5]);
	y2 = atof(argv[6]);
	segments = strtoul(argv[7], NULL, 10);
	printf("# line from %f %f to %f %f in %d segments\n", x1, y1, x2, y2, segments);

	d2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
	d = sqrtf(d2);
	segmentLen = d / (float)segments;

	for (i = 0; i <= segments; ++i) {
		x = x1 + (((float)i * segmentLen) / d) * (x2 - x1);
		y = y1 + (((float)i * segmentLen) / d) * (y2 - y1);
		kinematicsInverse(x, y, L1_MM, L2_MM, &S, &E);
		printf("%d\t%d\n", kinematicsRadToStep(S), kinematicsRadToStep(E));
	}
	
	return EXIT_SUCCESS;
}

int task_kinematics_arc(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}
