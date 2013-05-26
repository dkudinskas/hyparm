#ifndef __INSTRUCTION_EMU__SCANNER_H__
#define __INSTRUCTION_EMU__SCANNER_H__

#include "common/compiler.h"
#include "common/types.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"


#define SCANNER_CALL_SOURCE_NOT_SET              0
#define SCANNER_CALL_SOURCE_BOOT                 1
#define SCANNER_CALL_SOURCE_SVC                  2
#define SCANNER_CALL_SOURCE_DABT_PERMISSION      3
#define SCANNER_CALL_SOURCE_DABT_TRANSLATION     4
#define SCANNER_CALL_SOURCE_DABT_GVA_PERMISSION  5
#define SCANNER_CALL_SOURCE_PABT_FREERTOS        6
#define SCANNER_CALL_SOURCE_PABT_TRANSLATION     7
#define SCANNER_CALL_SOURCE_INTERRUPT            8


BasicBlock* scanBlock(GCONTXT *context, u32int startAddress);

u32int rescanBlock(GCONTXT *context, u32int blockStoreIndex, BasicBlock* block, u32int hostPC);

__macro__ u32int fetchThumbInstr(u16int *instructionPointer);
__macro__ bool txxIsThumb32(u32int instruction);

#ifdef CONFIG_SCANNER_COUNT_BLOCKS
void resetScanBlockCounter(void);
#else
#define resetScanBlockCounter()
#endif


#ifdef CONFIG_SCANNER_EXTRA_CHECKS
void setScanBlockCallSource(u8int source);
#else
#define setScanBlockCallSource(source)
#endif

__macro__ u32int fetchThumbInstr(u16int *instructionPointer)
{
  u16int halfWord = *instructionPointer;
  switch (halfWord & THUMB32)
  {
    case THUMB32_1:
    case THUMB32_2:
    case THUMB32_3:
      /*
       * 32-bit Thumb instruction; fetch and append next halfword
       */
      return (halfWord << 16) | *++instructionPointer;
    default:
      /*
       * 16-bit Thumb instruction
       * FIXME: check coverage of masks
       */
      return halfWord;
  }
}

/*
 * Checks whether an instruction word of a Thumb instruction is a Thumb-32 instruction.
 */
__macro__ bool txxIsThumb32(u32int instruction)
{
  return instruction & 0xFFFF0000;
}

#endif /* __INSTRUCTION_EMU__SCANNER_H__ */
