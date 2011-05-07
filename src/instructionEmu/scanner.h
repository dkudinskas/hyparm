#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/types.h"
#include "common/thumbdefs.h"
#include "guestManager/guestContext.h"


// uncomment me to enable scanner debug: #define SCANNER_DEBUG

void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

#endif
