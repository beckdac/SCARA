#ifndef __LASER_H__
#define __LASER_H__

#define LASER_DEFAULT_PIN 19

void laserInit(uint8_t pin);
void laserOn();
void laserOff();
void laserToggle();
void laserCleanup();
uint8_t laserGetPin(void);
uint8_t laserGetState(void);

#endif /* __LASER_H__ */
