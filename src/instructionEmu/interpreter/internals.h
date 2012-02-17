#ifndef __INSTRUCTION_EMU__INTERPRETER__INTERNALS_H__
#define __INSTRUCTION_EMU__INTERPRETER__INTERNALS_H__

/*
 * Functionality specific to the interpreter.
 *
 * DO NOT use or include this file outside the interpreter.
 */


#include "common/debug.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"


#define DEBUG_TRACE(what, context, instruction)                                                    \
  DEBUG(what, "%s: %#.8x @ %#.8x" EOL, __func__, context->R15, instruction);

#define TRACE(context, instruction)                                                                \
  printf("%s: %#.8x @ %#.8x" EOL, __func__, context->R15, instruction);


#define ARM_EXTRACT_CONDITION_CODE(instructionWord)  (instructionWord >> 28)


typedef enum
{
  ADD,
  ADC,
  SUB,
  SBC,
  RSB,
  RSC,
  AND,
  ORR,
  EOR,
  BIC,
  MOV,
  MVN,
  CMP,
  CMN,
  TST,
  TEQ,
  LSL,
  LSR,
  ASR,
  RRX,
  ROR
} OPTYPE;


u32int arithLogicOp(GCONTXT *context, u32int instr, OPTYPE opType, const char *instrString);

/* expand immediate12 field of instruction */
u32int armExpandImm12(u32int imm12);

// take shift type field from instr, return shift type
u32int decodeShift(u32int instrShiftType);

// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int *shamt);

/* a function to evaluate if a condition value is satisfied */
bool evaluateConditionCode(GCONTXT *context, u32int conditionCode);

void invalidDataProcTrap(GCONTXT *context, u32int instruction, const char *message)
  __attribute__((noinline,noreturn));

/* function to load a register value, evaluates modes. */
u32int loadGuestGPR(u32int regSrc, GCONTXT *context);

// rotate right function
u32int rorVal(u32int value, u32int ramt);

// generic any type shift function, changes input_parameter(carryFlag) value
u32int shiftVal(u32int imm32, u8int shiftType, u32int shamt, u8int *carryFlag);

/* function to store a register value, evaluates modes. */
void storeGuestGPR(u32int regDest, u32int value, GCONTXT *context);

#ifdef CONFIG_GUEST_TEST
// function to evaluate breakpoint value in unittests
void evalBkptVal(GCONTXT *context, u32int value);
#endif

#endif
