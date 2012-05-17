#ifndef __VM__OMAP_35XX__WDTIMER_H__
#define __VM__OMAP_35XX__WDTIMER_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


// register offsets and bit values
#define WDT_WIDR            0x00
#define WDT_WD_SYSCONFIG    0x10
#define WDT_WD_SYSSTATUS    0x14
#define WDT_WISR            0x18
#define WDT_WIER            0x1C
#define WDT_WCLR            0x24
#define WDT_WCRR            0x28
#define WDT_WLDR            0x2C
#define WDT_WTGR            0x30
#define WDT_WWPS            0x34
#define WDT_WSPR            0x48


void initWDTimer2(void) __cold__;
void resetWDTimer2(void);

u32int loadWDTimer2(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeWDTimer2(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);


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

#endif

