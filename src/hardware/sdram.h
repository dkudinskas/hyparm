#ifndef __SDRAM_H__
#define __SDRAM_H__

#include "types.h"
#include "serial.h"
#include "hardwareLibrary.h"

// uncomment me to enable sdram debug : #define SDRAM_DBG


u32int loadSdram(device * dev, ACCESS_SIZE size, u32int address);
void storeSdram(device * dev, ACCESS_SIZE size, u32int address, u32int value);


#endif

