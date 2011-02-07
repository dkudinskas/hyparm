#ifndef __BE_TIMER32K_H__
#define __BE_TIMER32K_H__

#include "types.h"
#include "serial.h"
#include "pageTable.h"

// uncomment me to enable debug : #define BE_TIMER32K_DBG

#define TIMER32K_BASE              0x48320000

#define REG_TIMER_32K_SYSCONFIG       0x0004
#define REG_TIMER_32K_COUNTER         0x0010

u32int beLoadTimer32k(u32int address);

void beInitTimer32k(void);

u32int getCounterVal(void);

#endif
