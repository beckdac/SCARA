#ifndef __COMPATABILITY_H__
#define __COMPATABILITY_H__

#include <sys/time.h>

#define _BV(bit) (1 << (bit))

#define printf_P printf
#define pgm_read_word(p) (*(p))
#define pgm_read_byte(p) (*(p))
#define PROGMEM __attribute__(( section(".progmem.data") ))
#define PSTR(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))

#if USE_GPIO
#define HIGH    GPIO_VALUE_HIGH
#define LOW     GPIO_VALUE_LOW
#else
#define HIGH    1
#define LOW     0
#endif

#endif /* __COMPATABILITY_H__ */
