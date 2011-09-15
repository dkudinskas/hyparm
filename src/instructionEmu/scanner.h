#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/types.h"

#ifdef CONFIG_THUMB2
#include "common/thumbdefs.h"
#endif

#include "guestManager/guestContext.h"


#define SCANNER_CALL_SOURCE_NOT_SET              0
#define SCANNER_CALL_SOURCE_BOOT                 1
#define SCANNER_CALL_SOURCE_SVC                  2
#define SCANNER_CALL_SOURCE_DABT_PERMISSION      3
#define SCANNER_CALL_SOURCE_DABT_TRANSLATION     4
#define SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION  5
#define SCANNER_CALL_SOURCE_PABT_FREERTOS        6
#define SCANNER_CALL_SOURCE_PABT_TRANSLATION     7


// uncomment me to enable scanner debug: #define SCANNER_DEBUG

#ifndef CONFIG_THUMB2

/*
 * TODO: why did Markos delete this?
 * Does this even belong in scanner.h?
 */
#define INSTR_SWI                 0xEF000000

#endif /* CONFIG_THUMB2 */

#if (CONFIG_DEBUG_SCANNER_CALL_SOURCE)
void setScanBlockCallSource(u8int source);
#else
#define setScanBlockCallSource(source)
#endif /* CONFIG_DEBUG_SCANNER_CALL_SOURCE */

void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

#if (CONFIG_DEBUG_SCANNER_COUNT_BLOCKS)
void resetScannerCounter(void);
#else
#define resetScannerCounter()
#endif /* CONFIG_DEBUG_SCANNER_COUNT_BLOCKS */

#endif
