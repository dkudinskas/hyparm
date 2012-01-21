#ifndef __DRIVERS__BEAGLE__BE_32K_TIMER_H__
#define __DRIVERS__BEAGLE__BE_32K_TIMER_H__

#include "common/types.h"


// uncomment me to enable debug : #define BE_TIMER32K_DBG

#define TIMER32K_BASE              0x48320000

#define REG_TIMER_32K_SYSCONFIG       0x0004
#define REG_TIMER_32K_COUNTER         0x0010

u32int beLoadTimer32k(u32int address);

void beInitTimer32k(void);

u32int getCounterVal(void);

void mdelay32k(u32int);

#endif
