#ifndef __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_
#define __INSTRUCTION_EMU__COMMON_INSTR_FUNCTIONS_H_

#include "common/types.h"

#include "guestManager/guestContext.h"


#define ARM_EXTRACT_CONDITION_CODE(instructionWord)  (instructionWord >> 28)

/*
 * Checks whether an instruction word of a Thumb instruction is a Thumb-32 instruction.
 */
#define TXX_IS_T32(instructionWord)                  (instructionWord & 0xFFFF0000)


/* a function to evaluate if a condition value is satisfied */
bool evaluateConditionCode(GCONTXT *context, u32int conditionCode);

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


#ifdef CONFIG_THUMB2

// decode thumb instruction into 32-bit format
u32int fetchThumbInstr(u16int * address);


#endif /* CONFIG_THUMB2 */


#endif
