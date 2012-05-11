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



/* This function will process ImmRegRSR instructions see comment above implementation in c-file for further details*/
u32int *standardImmRegRSR(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

/* This function is similar to standardImmRegRSR but for instructions that do not have a destReg*/
u32int *standardImmRegRSRNoDest(TranslationCache *tc, u32int *instructionAddr, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);


/* Function will return a register different from usedRegisterX
 * obviously, always returns 0, 1 or 2 for any 2 registers */
__macro__ u32int getOtherRegisterOf2(u32int usedRegister1, u32int usedRegister2)
{
  return countTrailingZeros(0xFFFF & ~((1 << usedRegister1) | (1 << usedRegister2)));
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__ */
