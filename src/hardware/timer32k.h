#ifndef __HARDWARE__TIMER_32K_H__
#define __HARDWARE__TIMER_32K_H__

#include "common/types.h"

#include "hardware/hardwareLibrary.h"


// uncomment me to enable debug : #define TIMER32K_DBG

#define TIMER32K_BASE              0x48320000

#define REG_TIMER_32K_SYSCONFIG       0x0004
#define REG_TIMER_32K_COUNTER         0x0010

u32int loadTimer32k(device * dev, ACCESS_SIZE size, u32int address);
void storeTimer32k(device * dev, ACCESS_SIZE size, u32int address, u32int value);

void initTimer32k(void);

#endif
