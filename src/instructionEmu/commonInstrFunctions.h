#ifndef __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_
#define __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_

#include "common/types.h"

#include "guestManager/guestContext.h"


#define ARM_EXTRACT_CONDITION_CODE(instructionWord)  (instructionWord >> 28)


extern inline __attribute__((always_inline,gnu_inline)) u32int getRealPC(GCONTXT *context)
{
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction;
#else
  return context->R15;
#endif
}


/* a function to serve as a dead-loop if we decode something invalid */
void invalidInstruction(u32int instr, const char *msg) __attribute__((noreturn));


#ifdef CONFIG_BLOCK_COPY
/* a function that sets 4 bits to zero starting at startbit (left bit is most significant) */
u32int zeroBits(u32int instruction, u32int startbit);

/* This function will save the PC that corresponds to the one that should be read by an instruction at instructionAddress to reg */
u32int* savePCInReg(GCONTXT * context, u32int * instructionAddress, u32int * currBlockCopyCacheAddr, u32int reg  );

/* This function will process ImmRegRSR instructions see comment above implementation in c-file for further details*/
u32int* standardImmRegRSR(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);

/* This function is similar to standardImmRegRSR but for instructions that do not have a destReg*/
u32int* standardImmRegRSRNoDest(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);

#endif


/* a function to evaluate if a condition value is satisfied */
bool evaluateConditionCode(GCONTXT *context, u32int conditionCode);

/* function to store a register value, evaluates modes. */
void storeGuestGPR(u32int regDest, u32int value, GCONTXT * context);


#ifdef CONFIG_BLOCK_COPY

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


/* function to obtain a register value, evaluates modes. */
u32int loadGuestGPR(u32int regSource, GCONTXT * context);

/* expand immediate12 field of instruction */
u32int armExpandImm12(u32int imm12);

// generic any type shift function, changes input_parameter(carryFlag) value
u32int shiftVal(u32int imm32, u8int shiftType, u32int shamt, u8int * carryFlag);

// rotate right function
u32int rorVal(u32int value, u32int ramt);

// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int * shamt);

// take shift type field from instr, return shift type
u32int decodeShift(u32int instrShiftType);


#ifdef CONFIG_THUMB2

// decode thumb instruction into 32-bit format
u32int fetchThumbInstr(u16int * address);

// check whether the instruction is thumb 16 or 32bit
bool isThumb32(u32int instr);

#endif /* CONFIG_THUMB2 */


#endif
