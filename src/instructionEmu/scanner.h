#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "types.h"
#include "serial.h"
#include "asm-dis.h"
#include "decoder.h"
#include "guestContext.h"

// uncomment me to enable scanner debug: #define SCANNER_DEBUG

// uncomment me to enable scanner request counter: #define DUMP_SCANNER_COUNTER

#define INSTR_SWI                 0xEF000000

void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

#ifdef DUMP_SCANNER_COUNTER
void resetScannerCounter(void);
#endif

#endif
