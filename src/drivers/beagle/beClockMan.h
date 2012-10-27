#ifndef __DRIVERS__BEAGLE__BE_CLOCK_MAN_H__
#define __DRIVERS__BEAGLE__BE_CLOCK_MAN_H__

#include "common/types.h"

#include "vm/omap35xx/clockManagerInternals.h"

void clkManBEInit(void);

u32int clkManRegReadBE(u32int module, u32int regOffs);
void clkManRegWriteBE(u32int module, u32int regOffs, u32int value);

void setClockSource(u32int clockID, bool sysClock);
void toggleTimerFclk(u32int clockID, bool enable);

void cmDisableDssClocks(void);


#endif
