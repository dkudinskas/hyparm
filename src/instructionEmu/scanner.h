#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me to enable scanner debug:
//#define SCANNER_DEBUG

// uncomment me to enable scanner debug for blockCopyCache: #define SCANNER_DEBUG_BLOCKCOPY

// uncomment me to enable scanner request counter: #define DUMP_SCANNER_COUNTER

//uncomment to dump scanner counter on DIE_NOW:
//#define DIE_NOW_SCANNER_COUNTER

#ifdef DIE_NOW_SCANNER_COUNTER
  #define SCANNER_COUNTER
  extern u32int scannerReqCounter; //from scanner.c
#endif

#ifdef DUMP_SCANNER_COUNTER
  #define SCANNER_COUNTER
#endif

#ifdef SCANNER_DEBUG_BLOCKCOPY
  #define SCANNER_DEBUG
#endif

#ifdef SCANNER_COUNTER
  extern u32int scannerReqCounter;
#endif

#define INSTR_SWI                 0xEF000000

void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

#ifdef CONFIG_BLOCK_COPY
u32int allSrcRegNonPC(u32int instruction);
#endif

#ifdef SCANNER_COUNTER
void resetScannerCounter(void);
#endif

#endif
