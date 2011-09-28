#ifndef __INSTRUCTION_EMU__DATA_PROCESS_INSTR__
#define __INSTRUCTION_EMU__DATA_PROCESS_INSTR__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for ARM instruction trace: #define DATA_PROC_TRACE

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

void invalidDataProcTrap(const char * msg, GCONTXT * gc);

u32int arithLogicOp(GCONTXT *context, u32int instruction, OPTYPE opType, const char *instrString);

// Arithmetic operations
u32int rsbInstruction(GCONTXT *context, u32int instruction);
u32int rscInstruction(GCONTXT *context, u32int instruction);
u32int subInstruction(GCONTXT *context, u32int instruction);
u32int sbcInstruction(GCONTXT *context, u32int instruction);
u32int addInstruction(GCONTXT *context, u32int instruction);
u32int adcInstruction(GCONTXT *context, u32int instruction);
// bitwise logic operations
u32int andInstruction(GCONTXT *context, u32int instruction);
u32int orrInstruction(GCONTXT *context, u32int instruction);
u32int eorInstruction(GCONTXT *context, u32int instruction);
u32int bicInstruction(GCONTXT *context, u32int instruction);
// bitwise compare operations
u32int tstInstruction(GCONTXT *context, u32int instruction);
u32int teqInstruction(GCONTXT *context, u32int instruction);
u32int cmpInstruction(GCONTXT *context, u32int instruction);
u32int cmnInstruction(GCONTXT *context, u32int instruction);
// mov operations
u32int movInstruction(GCONTXT *context, u32int instruction);
u32int mvnInstruction(GCONTXT *context, u32int instruction);
// shift register operations
u32int lslInstruction(GCONTXT *context, u32int instruction);
u32int lsrInstruction(GCONTXT *context, u32int instruction);
u32int asrInstruction(GCONTXT *context, u32int instruction);
u32int rrxInstruction(GCONTXT *context, u32int instruction);
u32int rorInstruction(GCONTXT *context, u32int instruction);

#endif
