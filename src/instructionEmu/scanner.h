#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/types.h"

#ifdef CONFIG_THUMB2
#include "common/thumbdefs.h"
#endif

#include "guestManager/guestContext.h"


// uncomment me to enable scanner debug:
//#define SCANNER_DEBUG

// uncomment me to enable scanner debug for blockCopyCache: #define SCANNER_DEBUG_BLOCKCOPY

#ifdef CONFIG_BLOCK_COPY

#ifdef SCANNER_DEBUG_BLOCKCOPY
  #define SCANNER_DEBUG
#endif

#endif

#ifndef CONFIG_THUMB2

/*
 * TODO: why did Markos delete this?
 * Does this even belong in scanner.h?
 */
#define INSTR_SWI                 0xEF000000

#endif /* CONFIG_THUMB2 */


void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

#ifdef CONFIG_BLOCK_COPY
u32int allSrcRegNonPC(u32int instruction);
#endif

#ifdef CONFIG_DEBUG_SCANNER_COUNT_BLOCKS

void resetScannerCounter(void);

#endif /* CONFIG_DEBUG_SCANNER_COUNT_BLOCKS */


#ifdef CONFIG_DEBUG_SCANNER_CALL_SOURCE

#define SCANNER_CALL_SOURCE_NOT_SET              0
#define SCANNER_CALL_SOURCE_BOOT                 1
#define SCANNER_CALL_SOURCE_SVC                  2
#define SCANNER_CALL_SOURCE_DABT_PERMISSION      3
#define SCANNER_CALL_SOURCE_DABT_TRANSLATION     4
#define SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION  5
#define SCANNER_CALL_SOURCE_PABT_FREERTOS        6
#define SCANNER_CALL_SOURCE_PABT_TRANSLATION     7


void setScanBlockCallSource(u8int source);

#endif /* CONFIG_DEBUG_SCANNER_CALL_SOURCE */

#endif
