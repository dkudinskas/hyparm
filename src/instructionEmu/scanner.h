#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


#define SCANNER_CALL_SOURCE_NOT_SET              0
#define SCANNER_CALL_SOURCE_BOOT                 1
#define SCANNER_CALL_SOURCE_SVC                  2
#define SCANNER_CALL_SOURCE_DABT_PERMISSION      3
#define SCANNER_CALL_SOURCE_DABT_TRANSLATION     4
#define SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION  5
#define SCANNER_CALL_SOURCE_PABT_FREERTOS        6
#define SCANNER_CALL_SOURCE_PABT_TRANSLATION     7


void scanBlock(GCONTXT * gc, u32int blkStartAddr);


#ifdef CONFIG_SCANNER_COUNT_BLOCKS

void resetScanBlockCounter(void);

#else

#define resetScanBlockCounter()

#endif /* CONFIG_DEBUG_SCANNER_COUNT_BLOCKS */


#ifdef CONFIG_SCANNER_EXTRA_CHECKS

void setScanBlockCallSource(u8int source);

#else

#define setScanBlockCallSource(source)

#endif /* CONFIG_SCANNER_EXTRA_CHECKS */

#endif /* __INSTRUCTION_EMU__SCANNER_H__ */
