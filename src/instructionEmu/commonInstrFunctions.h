#ifndef __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_
#define __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_

#include "common/types.h"

#include "guestManager/guestContext.h"


/* a function to serve as a dead-loop if we decode something invalid */
void invalidInstruction(u32int instr, const char *msg) __attribute__((noreturn));

/* a function to evaluate if guest is in priviledge mode or user mode */
bool guestInPrivMode(GCONTXT * context);

/* a function to evaluate if a condition value is satisfied */
bool evalCC(u32int instrCC, u32int cpsrCC);

/* function to store a register value, evaluates modes. */
void storeGuestGPR(u32int regDest, u32int value, GCONTXT * context);

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
