#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <poll.h>

#include "gpio.h"
#include "error.h"

gpio_t gpio = { 0 };

// GPIO_PARANOID does validation checks on most arguments even in performance critial code
#undef GPIO_PARANOID
// GPIO_VERBOSE provides verbose reporting on operations in both setup, shutdown and performance critial code
#undef GPIO_VERBOSE

void gpio_init(void) {
	if (gpio.init) {
		warning("gpio_init has already been called, skipping reconfigure\n");
		return;
	}
	memset(&gpio, 0, sizeof(gpio_t));

#ifdef GPIO_VERBOSE
	printf("opening export file '%s'\n", GPIO_EXPORT_FILENAME);
#endif
	gpio_open_file(GPIO_EXPORT_FILENAME, O_WRONLY, &gpio.export);
#ifdef GPIO_VERBOSE
	printf("opening unexport file '%s'\n", GPIO_UNEXPORT_FILENAME);
#endif
	gpio_open_file(GPIO_UNEXPORT_FILENAME, O_WRONLY, &gpio.unexport);

	gpio.init = 1;
}

void gpio_done(void) {
	uint8_t i;

#ifdef GPIO_VERBOSE
	printf("cleaning up gpio module\n");
#endif
	if (!gpio.init) {
		warning("gpio_done called before gpio_init\n");
		return;
	}

	for (i = 0; i < GPIO_PINS; ++i) {
		if (gpio.pin[i].init) {
#ifdef GPIO_VERBOSE
			printf("cleaning up pin %d\n", i);
#endif
			gpio_unexport(i);
		}
	}

	memset(&gpio, 0, sizeof(gpio_t));
}

void gpio_open_file(char *filename, int oflag,  gpio_file_t *gf) {
#ifdef GPIO_VERBOSE
	printf("opening file '%s' in mode %d\n", filename, oflag);
#endif
	memset(gf, 0, sizeof(gpio_file_t));
	gf->filename = strdup(filename);
	gf->oflag = oflag;
	gf->fd = open(filename, oflag);
	if (gf->fd < 0) {
		perror(NULL);
		fatal_error("unable to open GPIO file '%s' for writing\n", gf->filename);
	}
}

void gpio_open_pin_file(char *filename_template, uint8_t pin, int oflag, gpio_file_t *gf) {
	char buffer[GPIO_FILENAME_BUFFER_LENGTH];

	snprintf(buffer, GPIO_FILENAME_BUFFER_LENGTH, filename_template, pin);
	gpio_open_file(buffer, oflag, gf);
}

void gpio_close_file(gpio_file_t *gf) {
	int rv;

#ifdef GPIO_VERBOSE
	printf("closing file '%s'\n", gf->filename);
#endif
	rv = close(gf->fd);
	if (rv < 0) {
		perror(NULL);
		fatal_error("unable to close GPIO file '%s'\n", gf->filename);
	}
	free(gf->filename);
}

void gpio_export(uint8_t pin) {
	char buffer[GPIO_IO_BUFFER_LENGTH];
	ssize_t bl, rv;

#ifdef GPIO_VERBOSE
	printf("exporting GPIO pin %d to sysfs by writing '%d' to '%s'\n", pin, pin, gpio.export.filename);
#endif
	if (!gpio.init)
		fatal_error("gpio_export called before gpio_init (pin = %d)\n", pin);
	if (pin >= GPIO_PINS)
		fatal_error("invalid pin number passed to gpio_export (pin = %d)\n", pin);

	// setup the gpio directory for pin by writing to export file
	bl = snprintf(buffer, GPIO_IO_BUFFER_LENGTH, "%d", pin);
	// rewind to begining of file
	lseek(gpio.export.fd, 0, SEEK_SET);
	rv = write(gpio.export.fd, buffer, bl);
	if (rv < 0) {
#ifdef GPIO_PARANOID
		perror(NULL);
		fatal_error("unable to write %d bytes to export file '%s'\n", bl, gpio.export.filename);
#else
		warning("writing to '%s' to export pin %d failed... maybe already exported?\n", gpio.export.filename, pin);
#endif
	}

	// preopen the direction and value files
	gpio_open_pin_file(GPIO_DIR_FILENAME, pin, O_WRONLY, &gpio.pin[pin].dir);
	gpio_open_pin_file(GPIO_VALUE_FILENAME, pin, O_RDWR, &gpio.pin[pin].value);
	gpio.pin[pin].init = 1;
}

void gpio_unexport(uint8_t pin) {
	char buffer[GPIO_IO_BUFFER_LENGTH];
	ssize_t bl, rv;

#ifdef GPIO_VERBOSE
	printf("unexporting GPIO files to sysfs for pin %d\n", pin);
#endif
	if (!gpio.init)
		fatal_error("gpio_unexport called before gpio_init (pin = %d)\n", pin);
	if (pin >= GPIO_PINS)
		fatal_error("invalid pin number passed to gpio_unexport (pin = %d)\n", pin);

	// close the direction and value files
	gpio_close_file(&gpio.pin[pin].dir);
	gpio_close_file(&gpio.pin[pin].value);
	gpio.pin[pin].init = 0;

#ifdef GPIO_VERBOSE
	printf("finalizing unexport of pin %d by writing '%d' to '%s'\n", pin, pin, gpio.unexport.filename);
#endif
	bl = snprintf(buffer, GPIO_IO_BUFFER_LENGTH, "%d", pin);
	// rewind to begining of file
	lseek(gpio.unexport.fd, 0, SEEK_SET);
	rv = write(gpio.unexport.fd, buffer, bl);
	if (rv < 0) {
		perror(NULL);
		fatal_error("unable to write %d bytes to unexport file '%s'\n", bl, gpio.unexport.filename);
	}
}

void gpio_direction(uint8_t pin, uint8_t dir) {
	static const char dir_string[] = "in\0out";	// not a fan but fast
	ssize_t rv;

#ifdef GPIO_VERBOSE
	printf("setting pin %d direction to '%s' (%d)\n",  pin, (dir == GPIO_DIR_IN ? "in" : "out"), dir);
#endif
#ifdef GPIO_PARANOID
	if (!gpio.init)
		fatal_error("gpio_direction called before gpio_init (pin = %d)\n", pin);
	if (pin >= GPIO_PINS)
		fatal_error("invalid pin number passed to gpio_direction (pin = %d)\n", pin);
	if (!gpio.pin[pin].init)
		fatal_error("pin %d is not initialized (exported) before call to gpio_direction\n", pin);
#endif

	// rewind to begining of file
	lseek(gpio.pin[pin].dir.fd, 0, SEEK_SET);
	// write the direction
	rv = write(gpio.pin[pin].dir.fd, &dir_string[(dir == GPIO_DIR_IN ? 0 : 3)], (dir == GPIO_DIR_IN ? 2 : 3));
	if (rv < 0) {
		perror(NULL);
		fatal_error("unable to write %d bytes to direction file '%s'\n", (dir == GPIO_DIR_IN ? 2 : 3), gpio.pin[pin].dir.filename);
	}
}

int gpio_read(uint8_t pin) {
	static char value[GPIO_IO_VALUE_LEN];
	ssize_t rv;

#ifdef GPIO_VERBOSE
	printf("reading from pin %d via file '%s'\n", pin, gpio.pin[pin].value.filename);
#endif
#ifdef GPIO_PARANOID
	if (!gpio.init)
		fatal_error("gpio_read called before gpio_init (pin = %d)\n", pin);
	if (pin >= GPIO_PINS)
		fatal_error("invalid pin number passed to gpio_read (pin = %d)\n", pin);
	if (!gpio.pin[pin].init)
		fatal_error("pin %d is not initialized (exported) before call to gpio_read\n", pin);
#endif

	// rewind to begining of file
	lseek(gpio.pin[pin].value.fd, 0, SEEK_SET);
	// read the value
	rv = read(gpio.pin[pin].value.fd, value, GPIO_IO_VALUE_LEN);
	if (rv < 0) {
		perror(NULL);
		fatal_error("unable to read from GPIO value file '%s'\n", gpio.pin[pin].value.filename);
	}
#ifdef GPIO_VERBOSE
	printf("read '%d' for pin %d from file '%s'\n", atoi(value), pin, gpio.pin[pin].value.filename);
#endif

	return atoi(value);
}

void gpio_write(uint8_t pin, uint8_t value) {
	static const char values[] = "01";
	ssize_t rv;

#ifdef GPIO_VERBOSE
	printf("writing '%d' for pin %d to file '%s'\n", value, pin, gpio.pin[pin].value.filename);
#endif
#ifdef GPIO_PARANOID
	if (!gpio.init)
		fatal_error("gpio_write called before gpio_init (pin = %d)\n", pin);
	if (pin >= GPIO_PINS)
		fatal_error("invalid pin number passed to gpio_write (pin = %d)\n", pin);
	if (!gpio.pin[pin].init)
		fatal_error("pin %d is not initialized (exported) before call to gpio_write\n", pin);
#endif

	// rewind to begining of file
	lseek(gpio.pin[pin].value.fd, 0, SEEK_SET);
	// write the value
	rv = write(gpio.pin[pin].value.fd, &values[(value == GPIO_VALUE_LOW ? 0 : 1)], 1);
	if (rv < 0) {
		perror(NULL);
		fatal_error("unable to write to GPIO value file '%s'\n", gpio.pin[pin].value.filename);
	}
}

int gpio_poll(uint8_t pin, int timeout) {
	struct pollfd pfd;
	int rv;

#ifdef GPIO_VERBOSE
	if (timeout > 0)
		printf("polling pin %d from file '%s' for %d ms\n", pin, gpio.pin[pin].value.filename, timeout);
	else if (timeout == 0)
		printf("polling pin %d from file '%s' in non-blocking mode\n", pin, gpio.pin[pin].value.filename);
	else
		printf("polling pin %d from file '%s' in blocking mode (wait forever)\n", pin, gpio.pin[pin].value.filename);
#endif
#ifdef GPIO_PARANOID
	if (!gpio.init)
		fatal_error("gpio_poll called before gpio_init (pin = %d)\n", pin);
	if (pin >= GPIO_PINS)
		fatal_error("invalid pin number passed to gpio_poll (pin = %d)\n", pin);
	if (!gpio.pin[pin].init)
		fatal_error("pin %d is not initialized (exported) before call to gpio_poll\n", pin);
#endif

	pfd.fd = gpio.pin[pin].value.fd;
	pfd.events = POLLPRI;

	// to clear events on fd, must read current state first
	gpio_read(pin);

	// rewind to begining of file
	lseek(gpio.pin[pin].value.fd, 0, SEEK_SET);
	rv = poll(&pfd, 1, timeout);
	if (rv < 0) {
		perror(NULL);
		fatal_error("polling pin %d via file '%s' failed\n", pin, gpio.pin[pin].value.filename);
	}

#ifdef GPIO_VERBOSE
	printf("poll on pin %d via file '%s' returned %d\n", pin, gpio.pin[pin].value.filename, rv);
#endif

	return rv;
}
