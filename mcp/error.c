#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void fatal_error(const char * fmt, ...) {
	va_list vargs;
	va_start(vargs, fmt);
	vfprintf(stderr, fmt, vargs);
	va_end(vargs);

	exit(EXIT_FAILURE);
}

void warning(const char *fmt, ...) {
	va_list vargs;
	va_start(vargs, fmt);
	vfprintf(stderr, fmt, vargs);
	va_end(vargs);
}
