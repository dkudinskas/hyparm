#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "types.h"
#include "serial.h"
#include "asm-dis.h"
#include "decoder.h"
#include "guestContext.h"

// uncomment me to enable scanner debug: #define SCANNER_DEBUG

// uncomment me to enable scanner request counter:
#define DUMP_SCANNER_COUNTER

// uncomment me to enable output of number of scanned blocks by DIE_NOW also define #define DIE_NOW_SCANNER_COUNTER in debug.h
#define SCANNER_COUNTER

#ifdef DUMP_SCANNER_COUNTER
  #define SCANNER_COUNTER
#endif


#ifdef SCANNER_COUNTER
  extern u32int scannerReqCounter;
#endif


/* SWI */
#define INSTR_SWI                 0xEF000000

void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

u32int allSrcRegNonPC(u32int instruction);

#ifdef SCANNER_COUNTER
void resetScannerCounter(void);
#endif

#endif
