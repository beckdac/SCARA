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
#include "gpio.h"
#include "task.h"

int main(int argc, char **argv) {
	int task_exit;

	task_exit = task_parse(1, argc, argv);

	exit(task_exit);
}
