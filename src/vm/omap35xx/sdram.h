#ifndef __HARDWARE__SDRAM_H__
#define __HARDWARE__SDRAM_H__

#include "common/types.h"

#include "hardware/hardwareLibrary.h"
#include "hardware/serial.h"


#define MEGABYTE_COUNT   4096

// uncomment me to enable store counters:
#define SDRAM_STORE_COUNTER

// uncomment me to enable sdram debug : #define SDRAM_DBG


void initSdram(void);

u32int loadSdram(device * dev, ACCESS_SIZE size, u32int address);
void storeSdram(device * dev, ACCESS_SIZE size, u32int address, u32int value);

void dumpSdramStats(void);

struct SdramController
{
  u32int enabled;
#ifdef SDRAM_STORE_COUNTER
  u32int * storeCounters;
#endif
};

#endif

