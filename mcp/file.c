#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include "error.h"
#include "file.h"

file_t *file_open(const char *filename, const char *mode) {
	file_t *f;

	f = malloc(sizeof(file_t));
	memset(f, 0, sizeof(file_t));

	f->filename = strdup(filename);
	f->mode = strdup(mode);

	if ((f->fp = fopen(f->filename, f->mode)) == NULL) {
		perror(NULL);
		fatal_error("unable to open file '%s'\n", f->filename);
	}

	return f;
}

void file_close(file_t *f) {
	free(f->filename);
	free(f->mode);
	fclose(f->fp);
	free(f);
}
