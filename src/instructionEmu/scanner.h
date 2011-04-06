#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

// uncomment me to enable scanner debug: #define SCANNER_DEBUG

#define INSTR_SWI                 0xEF000000

void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

#endif
