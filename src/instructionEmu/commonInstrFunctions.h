#ifndef __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_
#define __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_

#include "common/types.h"

#include "guestManager/guestContext.h"


#ifdef CONFIG_BLOCK_COPY
/* a function that sets 4 bits to zero starting at startbit (left bit is most significant) */
u32int zeroBits(u32int instruction, u32int startbit);

/* This function will save the PC that corresponds to the one that should be read by an instruction at instructionAddress to reg */
u32int* savePCInReg(GCONTXT * context, u32int * instructionAddress, u32int * currBlockCopyCacheAddr, u32int reg  );

/* This function will process ImmRegRSR instructions see comment above implementation in c-file for further details*/
u32int* standardImmRegRSR(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);

/* This function is similar to standardImmRegRSR but for instructions that do not have a destReg*/
u32int* standardImmRegRSRNoDest(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);

/* function to find a register that is not one of the arguments */
u32int findUnusedRegister(u32int regSrc1, u32int regDest, u32int regSrc2);

/* This function inserts an instruction in the instructionstream of the blockCopycache which will write the content of reg2Backup to the reserved word*
 * The reserved word = a word in the blockCopyCache that won't contain instructions (if present it is situated right after the backpointer)
 * If there isn't a free word to store the backup than blockCopyCacheStartAddress will end with a zero otherwise with a one
 * If no free word is available backupRegister should leave a blank word
 */
u32int * backupRegister(u32int reg2Backup, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);

/* This function inserts an instruction in the instructionstream of the blockCopycache which will restore the content of reg2Restore from the reserved word*
 * The reserved word = a word in the blockCopyCache that won't contain instructions (if present it is situated right after the backpointer)
 * The last bit of blockCopyCacheStartAddress can safely be ignored
 */
u32int * restoreRegister(u32int reg2Restore, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);

#endif

#ifdef CONFIG_THUMB2

// decode thumb instruction into 32-bit format
u32int fetchThumbInstr(u16int * address);

#endif /* CONFIG_THUMB2 */

#endif
