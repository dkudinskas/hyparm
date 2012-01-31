#ifndef __VM__OMAP_35XX__LED_H__
#define __VM__OMAP_35XX__LED_H__

#include "common/types.h"


#define LED1            0x00200000
#define LED2            0x00400000
#define DATA_SET_REG    0x49056094
#define DATA_CLEAR_REG  0x49056090

int turnOff(void);
int turnOn(void);

#endif
