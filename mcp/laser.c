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
#include "compatability.h"
#include "stepper.h"
#include "util.h"
#include "laser.h"

static uint8_t laserPin = LASER_DEFAULT_PIN;
static uint8_t laserState = 0;

void laserInit(uint8_t pin) {
	laserPin = pin;

	gpio_export(laserPin);
        gpio_direction(laserPin, GPIO_DIR_OUT);
	gpio_write(laserPin, 0);
}

void laserOn(void) {
	laserState = 1;
	gpio_write(laserPin, 1);
}

void laserOff(void) {
	laserState = 0;
	gpio_write(laserPin, 0);
}

uint8_t laserGetPin(void) {
	return laserPin;
}

uint8_t laserGetState(void) {
	return laserState;
}

void laserToggle(void) {
	if (laserState)
		laserOff();
	else
		laserOn();
}

void laserCleanup(void) {
	laserOff();
	gpio_unexport(laserPin);
}
