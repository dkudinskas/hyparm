#ifndef __VM__OMAP_35XX__SDRAM_H__
#define __VM__OMAP_35XX__SDRAM_H__

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/guestContext.h"

#include "vm/types.h"


#define MEGABYTE_COUNT   4096


struct SdramController
{
  u32int enabled;
#ifdef CONFIG_SDRAM_STORE_COUNTER
  u32int *storeCounters;
#endif
};


void dumpSdramStats(struct SdramController *sdram);
void initSdram(virtualMachine *vm) __cold__;
u32int loadSdram(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSdram(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__SDRAM_H__ */
