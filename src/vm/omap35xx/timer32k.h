#ifndef __HARDWARE__TIMER_32K_H__
#define __HARDWARE__TIMER_32K_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


#define TIMER32K_BASE              0x48320000

#define REG_TIMER_32K_SYSCONFIG       0x0004
#define REG_TIMER_32K_COUNTER         0x0010

u32int loadTimer32k(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeTimer32k(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

void initTimer32k(virtualMachine *vm) __cold__;

struct SynchronizedTimer32k
{
  u32int timer32SysconfReg;
  u32int counterVal;
};

#endif
