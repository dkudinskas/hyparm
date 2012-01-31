#ifndef __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_
#define __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_

#include "common/types.h"

#include "guestManager/guestContext.h"


/****************** all defines for CPSR (spsr's) ************/
/* mode field / bits */
#define CPSR_MODE_FIELD    0x0000001F
#define CPSR_MODE_USER       0x10
#define CPSR_MODE_FIQ        0x11
#define CPSR_MODE_IRQ        0x12
#define CPSR_MODE_SVC        0x13
#define CPSR_MODE_ABORT      0x17
#define CPSR_MODE_UNDEF      0x1B
#define CPSR_MODE_SYSTEM     0x1F
/* thumb mode bit, IRQ and FIQ bits */
#define CPSR_THUMB_BIT     0x00000020
#define CPSR_FIQ_BIT       0x00000040
#define CPSR_IRQ_BIT       0x00000080
#define CPSR_AAB_BIT       0x00000100
/* random flags.. */
#define CPSR_CUM_SAT_FLAG     0x08000000
#define CPSR_IF_THEN_0_1      0x06000000
#define CPSR_JAZELLE_MODE     0x01000000
#define CPSR_RESERVED         0x00f00000
#define CPSR_GTE_FLAGS_SIMD   0x000f0000
#define CPSR_IF_THEN_2_7      0x0000fc00
#define CPSR_ENDIANNESS       0x00000200 
/* ARM instruction condition field / values */
#define CPSR_CC_FIELD      0xF0000000
#define CC_EQ   0x0  // equals
#define CC_NE   0x1  // not equals
#define CC_HS   0x2  // carry set / unsigned higher or same
#define CC_LO   0x3  // carry not set / unsigned lower
#define CC_MI   0x4  // minus / negative / N set
#define CC_PL   0x5  // plus / positive or zero / N clear
#define CC_VS   0x6  // overflow / V set
#define CC_VC   0x7  // no overflow / V clear
#define CC_HI   0x8  // unsigned higher
#define CC_LS   0x9  // unsigned lower or same
#define CC_GE   0xA  // signed greater than or equals
#define CC_LT   0xB  // signed less than
#define CC_GT   0xC  // signed greater than
#define CC_LE   0xD  // signed less than or equal
#define CC_AL   0xE  // always
#define CC_NV   0xF  // never - should not be used! only special uncond instr
/* flag bits in CPSR */
#define CC_N_FLAG  0x8  // negative flag
#define CC_Z_FLAG  0x4  // zero flag
#define CC_C_FLAG  0x2  // carry flag
#define CC_V_FLAG  0x1  // oVerflow flag

#define SHIFT_TYPE_LSL    0x0
#define SHIFT_TYPE_LSR    0x1
#define SHIFT_TYPE_ASR    0x2
#define SHIFT_TYPE_RRX    0x3
#define SHIFT_TYPE_ROR    0x4

/* a function to serve as a dead-loop if we decode something invalid */
void invalidInstruction(u32int instr, const char * msg) __attribute__((noreturn));


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


/* a function to evaluate if guest is in priviledge mode or user mode */
bool guestInPrivMode(GCONTXT * context);

/* a function to evaluate if a condition value is satisfied */
bool evalCC(u32int instrCC, u32int cpsrCC);

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

// count the number of ones in a 32 bit stream
u32int countBitsSet(u32int bitstream);


#ifdef CONFIG_THUMB2

// decode thumb instruction into 32-bit format
u32int fetchThumbInstr(u16int * address);

// check whether the instruction is thumb 16 or 32bit
bool isThumb32(u32int instr);

#endif /* CONFIG_THUMB2 */


#endif
