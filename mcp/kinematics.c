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

int kinematics_forward(float *x, float *y, float L1, float L2, float S, float E) {
	if (L1 <= 0 || L2 <= 0)
		return 0;
	*x = L1 * cosf(S) + L2 * cosf(S + E);
	*y = L1 * sinf(S) + L2 * sinf(S + E);
	return 1;
}

int kinematics_inverse(float x, float y, float L1, float L2, float *S, float *E) {
	if (L1 <= 0 || L2 <= 0)
		return 0;
	*E = acosf((x*x + y*y - L1*L1 - L2*L2)/(2. * L1 * L2));
	*S = atan2f(y, x) - acosf((x*x + y*y + L1*L1 - L2*L2)/(2*L1 * sqrtf(x*x + y*y)));
	return 1;
}

int kinematics_test(int argc, char *argv[]) {
	float x, y, L1, L2, S, E;

	x = 0.5;
	y = 0.46;
	L1 = .4;
	L2 = .3;
	printf("x = %.2f   y = %.2f   L1 = %.2f   L2 = %.2f\n", x, y, L1, L2);
	kinematics_inverse(x, y, L1, L2, &S, &E);
	printf("S = %.4f   E = %.4f\n", S * 180. / M_PI, E * 180. / M_PI);
	kinematics_forward(&x, &y, L1, L2, S, E);
	printf("x = %.4f   y = %.4f\n", x, y);

	return EXIT_SUCCESS;
}
