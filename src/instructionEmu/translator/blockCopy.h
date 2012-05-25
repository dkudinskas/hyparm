#ifndef __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__
#define __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__

#include "common/bit.h"
#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/translationInfo.h"


__macro__ void addPCMapping(ARMTranslationInfo *block, PCRemapAction pcRemapAction);

void armBackupRegisterToSpill(TranslationCache *tc, ARMTranslationInfo *block, u32int conditionCode, u32int reg);
void armRestoreRegisterFromSpill(TranslationCache *tc, ARMTranslationInfo *block, u32int conditionCode, u32int reg);
void armWritePCToRegister(TranslationCache *tc, ARMTranslationInfo *block, u32int conditionCode, u32int reg, u32int pc);

__macro__ void armWriteToCodeCache(TranslationCache *tc, ARMTranslationInfo *block,
                                   u32int instruction);

/* function to find a register that is not one of the arguments */
__macro__ u32int getOtherRegister(u32int usedRegister);
__macro__ u32int getOtherRegisterOf2(u32int usedRegister1, u32int usedRegister2);
__macro__ u32int getOtherRegisterOf3(u32int usedRegister1, u32int usedRegister2, u32int usedRegister3);

__macro__ void setPCMapping(ARMTranslationInfo *block, PCRemapAction pcRemapAction);


__macro__ void addPCMapping(ARMTranslationInfo *block, PCRemapAction pcRemapAction)
{
  setPCMapping(block, pcRemapAction);
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
}

__macro__ void armWriteToCodeCache(TranslationCache *tc, ARMTranslationInfo *block,
                                   u32int instruction)
{
  block->code = updateCodeCachePointer(tc, block->code);
  *block->code = instruction;
  block->code++;
  block->pcRemapBitmapShift += PC_REMAP_BIT_COUNT;
}

__macro__ u32int getOtherRegister(u32int usedRegister)
{
  return !usedRegister;
}

__macro__ u32int getOtherRegisterOf2(u32int usedRegister1, u32int usedRegister2)
{
  return countTrailingZeros(~((1 << usedRegister1) | (1 << usedRegister2)));
}

__macro__ u32int getOtherRegisterOf3(u32int usedRegister1, u32int usedRegister2, u32int usedRegister3)
{
  return countTrailingZeros(~((1 << usedRegister1) | (1 << usedRegister2) | (1 << usedRegister3)));
}

__macro__ void setPCMapping(ARMTranslationInfo *block, PCRemapAction pcRemapAction)
{
  /*
   * The PC remapping bitmap is an array of 32-bit words. The current position in the bitmap is
   * determined as follows:
   *
   * word index | 5-bit shift (last bit expected to be zero, since PC_REMAP_BIT_COUNT == 2)
   */
  ASSERT(block->pcRemapBitmapShift < PC_REMAP_BITMAP_SIZE_BITS, ERROR_NOT_IMPLEMENTED);
  block->metaEntry.pcRemapBitmap[block->pcRemapBitmapShift >> 5]
                                |= pcRemapAction << (block->pcRemapBitmapShift & 0x1f);
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__ */
