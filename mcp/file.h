#ifndef _FILE_H_
#define _FILE_H_

typedef struct file {
	char *filename;
	char *mode;
	FILE *fp;
} file_t;

file_t *file_open(const char *filename, const char *mode);
void file_close(file_t *f);

#endif /* _FILE_H_ */
