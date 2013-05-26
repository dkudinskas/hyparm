#ifndef __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__
#define __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__

#include "common/bit.h"
#include "common/types.h"

#include "guestManager/guestContext.h"


void armSpillRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg, u32int tempReg);
void armRestoreRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg);
void armWritePCToRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg, u32int pc);
void armWriteSpillLocToRegister(TranslationStore* ts, BasicBlock* block, u32int conditionCode, u32int reg);

/* function to find a register that is not one of the arguments */
__macro__ u32int getOtherRegister(u32int usedRegister);
__macro__ u32int getOtherRegisterOf2(u32int usedRegister1, u32int usedRegister2);
__macro__ u32int getOtherRegisterOf3(u32int usedRegister1, u32int usedRegister2, u32int usedRegister3);

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

#endif /* __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__ */
