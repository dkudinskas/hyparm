#ifndef __DRIVERS__BEAGLE__BE_32K_TIMER_H__
#define __DRIVERS__BEAGLE__BE_32K_TIMER_H__

#include "common/types.h"


u32int getCounterVal(void);

void mdelay32k(u32int);

#endif
