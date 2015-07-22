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
#include "stepper.h"
#include "ui.h"
#include "util.h"
#include "limits.h"
#include "core.h"
#include "kinematics.h"

int kinematicsForward(float *x, float *y, float L1, float L2, float S, float E) {
	if (L1 <= 0 || L2 <= 0)
		return 0;
	*x = L1 * cosf(S) + L2 * cosf(S + E);
	*y = L1 * sinf(S) + L2 * sinf(S + E);
	return 1;
}

int kinematicsInverse(float x, float y, float L1, float L2, float *S, float *E) {
	if (L1 <= 0 || L2 <= 0)
		return 0;
	*E = acosf((x*x + y*y - L1*L1 - L2*L2)/(2. * L1 * L2));
	*S = atan2f(y, x) - acosf((x*x + y*y + L1*L1 - L2*L2)/(2*L1 * sqrtf(x*x + y*y)));
	return 1;
}

float kinematicsStepToRad(int steps) {
	return (float)steps / STEPS_PER_REV * (2. * M_PI);
}

int kinematicsRadToStep(float rad) {
	return (int)lroundf(rad / (2. * M_PI) * STEPS_PER_REV);
}

float kinematicsRadToDeg(float rad) {
	return rad * 180. / M_PI;
}

float kinematicsDegToRad(float deg) {
	return deg * M_PI / 180.;
}

int kinematicsTest(int argc, char *argv[]) {
	float x, y, L1, L2, S, E;

	if (argc != 4) {
		int i;
KINEMATICS_TEST_USAGE:
		warning("usage: %s %s kinematics <X> <Y>\n", argv[0], argv[1]);
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
