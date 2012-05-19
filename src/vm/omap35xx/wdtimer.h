#ifndef __VM__OMAP_35XX__WDTIMER_H__
#define __VM__OMAP_35XX__WDTIMER_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct WatchdogTimer
{
  u32int widr;
  u32int wdSysconfig;
  u32int wdSysstatus;
  u32int wisr;
  u32int wier;
  u32int wclr;
  u32int wcrr;
  u32int wldr;
  u32int wtgr;
  u32int wwps;
  u32int wspr;
};


void initWDTimer2(virtualMachine *vm) __cold__;
u32int loadWDTimer2(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeWDTimer2(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__WDTIMER_H__ */

