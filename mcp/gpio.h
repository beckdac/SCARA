#ifndef _GPIO_H_
#define _GPIO_H_

#define GPIO_PINS	54

#define GPIO_IO_BUFFER_LENGTH	5
#define GPIO_IO_VALUE_LEN	3
#define GPIO_FILENAME_BUFFER_LENGTH 128

#define GPIO_EXPORT_FILENAME	"/sys/class/gpio/export"
#define GPIO_UNEXPORT_FILENAME	"/sys/class/gpio/unexport"

#define GPIO_DIR_FILENAME	"/sys/class/gpio/gpio%d/direction"
#define GPIO_DIR_IN		0
#define GPIO_DIR_IN_STR		"in"
#define GPIO_DIR_OUT		1
#define GPIO_DIR_OUT_STR	"out"

#define GPIO_VALUE_FILENAME	"/sys/class/gpio/gpio%d/value"
#define GPIO_VALUE_LOW		0
#define GPIO_VALUE_LOW_STR	"0"
#define GPIO_VALUE_HIGH		1
#define GPIO_VALUE_HIGH_STR	"1"

#define GPIO_POLL_FOREVER	-1

typedef struct gpio_pin_config_file {
	char *filename;
	int oflag;
	int fd;
} gpio_file_t;

typedef struct gpio_pin_config {
	uint8_t	init;
	gpio_file_t dir;
	gpio_file_t value;
} gpio_pin_t;

typedef struct gpio_config {
	uint8_t init;
	gpio_file_t export;
	gpio_file_t unexport;
	gpio_pin_t pin[GPIO_PINS];
} gpio_t;

void gpio_init(void);
void gpio_done(void);
void gpio_open_file(char *filename, int oflag,  gpio_file_t *gf);
void gpio_open_pin_file(char *filename_template, uint8_t pin, int oflag, gpio_file_t *gf);
void gpio_close_file(gpio_file_t *gf);
void gpio_export(uint8_t pin);
void gpio_unexport(uint8_t pin);
void gpio_direction(uint8_t pin, uint8_t dir);
int gpio_read(uint8_t pin);
void gpio_write(uint8_t pin, uint8_t value);
int gpio_poll(uint8_t pin, int timeout);

#endif /* _GPIO_H_ */
