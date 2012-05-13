#ifndef __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__
#define __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__

#include "common/bit.h"
#include "common/types.h"

#include "guestManager/guestContext.h"
#include "guestManager/translationCache.h"


u32int *armBackupRegisterToSpill(TranslationCache *tc, u32int *code, u32int conditionCode, u32int reg);
u32int *armRestoreRegisterFromSpill(TranslationCache *tc, u32int *code, u32int conditionCode, u32int reg);
u32int *armWritePCToRegister(TranslationCache *tc, u32int *code, u32int conditionCode, u32int reg, u32int pc);

/* function to find a register that is not one of the arguments */
__macro__ u32int getOtherRegisterOf2(u32int usedRegister1, u32int usedRegister2);
__macro__ u32int getOtherRegisterOf3(u32int usedRegister1, u32int usedRegister2, u32int usedRegister3);


/* Function will return a register different from usedRegisterX
 * obviously, always returns 0, 1 or 2 for any 2 registers */
__macro__ u32int getOtherRegisterOf2(u32int usedRegister1, u32int usedRegister2)
{
  return countTrailingZeros(0xFFFF & ~((1 << usedRegister1) | (1 << usedRegister2)));
}
/* returns 0,1,2,3 for any 3... DUH */
__macro__ u32int getOtherRegisterOf3(u32int usedRegister1, u32int usedRegister2, u32int usedRegister3)
{
  return countTrailingZeros(0xFFFF & ~((1 << usedRegister1) | (1 << usedRegister2) | (1 << usedRegister3)));
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__ */
