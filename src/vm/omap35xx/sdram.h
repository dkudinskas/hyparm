#ifndef __VM__OMAP_35XX__SDRAM_H__
#define __VM__OMAP_35XX__SDRAM_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


#define MEGABYTE_COUNT   4096



typedef struct EmumulatedVirtualMachine virtualMachine;


void initSdram(virtualMachine *vm) __cold__;

u32int loadSdram(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSdram(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

void dumpSdramStats(void);

struct SdramController
{
  u32int enabled;
#ifdef CONFIG_SDRAM_STORE_COUNTER
  u32int *storeCounters;
#endif
};

#endif

