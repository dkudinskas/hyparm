#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

// uncomment me to enable scanner debug: #define SCANNER_DEBUG

#define INSTR_SWI                 0xEF000000
#define INSTR_SWI_THUMB_MIX		  0xDF00BF00 // Create a 32-bit instruction combining a SVC|NOP
#define INSTR_SWI_THUMB			  0xDF00
#define INSTR_NOP_THUMB		      0xBF00

#define T_BIT					  0x20
void scanBlock(GCONTXT * gc, u32int blkStartAddr);

void protectScannedBlock(u32int startAddress, u32int endAddress);

#endif
