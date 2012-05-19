#ifndef __VM__OMAP35XX__TIMER_32K_H__
#define __VM__OMAP35XX__TIMER_32K_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct SynchronizedTimer32k
{
  u32int timer32SysconfReg;
  u32int counterVal;
};

void initTimer32k(virtualMachine *vm) __cold__;
u32int loadTimer32k(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeTimer32k(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP35XX__TIMER_32K_H__ */
