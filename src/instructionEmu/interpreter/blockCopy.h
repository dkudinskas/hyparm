#ifndef __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__
#define __INSTRUCTION_EMU__INTERPRETER__BLOCK_COPY_H__

#include "common/bit.h"
#include "common/types.h"

#include "guestManager/guestContext.h"
#include "guestManager/translationCache.h"


/* This function inserts an instruction in the instructionstream of the blockCopycache which will write the content of reg2Backup to the reserved word*
 * The reserved word = a word in the blockCopyCache that won't contain instructions (if present it is situated right after the backpointer)
 * If there isn't a free word to store the backup than blockCopyCacheStartAddress will end with a zero otherwise with a one
 * If no free word is available backupRegister should leave a blank word
 */
u32int *backupRegister(TranslationCache *tc, u32int reg2Backup, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

/* function to find a register that is not one of the arguments */
__macro__ u32int getOtherRegisterOf2(u32int usedRegister1, u32int usedRegister2);

/* This function inserts an instruction in the instructionstream of the blockCopycache which will restore the content of reg2Restore from the reserved word*
 * The reserved word = a word in the blockCopyCache that won't contain instructions (if present it is situated right after the backpointer)
 * The last bit of blockCopyCacheStartAddress can safely be ignored
 */
u32int *restoreRegister(TranslationCache *tc, u32int reg2Restore, u32int *currBlockCopyCacheAddr, u32int *blockCopyCacheStartAddress);

/* This function will save the PC that corresponds to the one that should be read by an instruction at instructionAddress to reg */
u32int *savePCInReg(TranslationCache *tc, u32int *instructionAddress, u32int *currBlockCopyCacheAddr, u32int reg);

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
