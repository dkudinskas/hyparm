#ifndef __LED_H__
#define __LED_H__

#include "types.h"

#define LED1  				0x00200000
#define LED2  				0x00400000
#define DATA_SET_REG		0x49056094
#define DATA_CLEAR_REG		0x49056090

int turnOff(void);
int turnOn(void);

#endif