#ifndef __VM__OMAP_35XX__SDRAM_H__
#define __VM__OMAP_35XX__SDRAM_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


#define MEGABYTE_COUNT   4096

// uncomment me to enable store counters:
#define SDRAM_STORE_COUNTER


void initSdram(void);

u32int loadSdram(device * dev, ACCESS_SIZE size, u32int address);
void storeSdram(device * dev, ACCESS_SIZE size, u32int address, u32int value);

void dumpSdramStats(void);

struct SdramController
{
  u32int enabled;
#ifdef SDRAM_STORE_COUNTER
  u32int *storeCounters;
#endif
};

#endif

