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

u32int arithLogicOp(GCONTXT * context, OPTYPE opType, const char * instrString);

// Arithmetic operations
u32int rsbInstruction(GCONTXT * context);
u32int rscInstruction(GCONTXT * context);
u32int subInstruction(GCONTXT * context);
u32int sbcInstruction(GCONTXT * context);
u32int addInstruction(GCONTXT * context);
u32int adcInstruction(GCONTXT * context);
// bitwise logic operations
u32int andInstruction(GCONTXT * context);
u32int orrInstruction(GCONTXT * context);
u32int eorInstruction(GCONTXT * context);
u32int bicInstruction(GCONTXT * context);
// bitwise compare operations
u32int tstInstruction(GCONTXT * context);
u32int teqInstruction(GCONTXT * context);
u32int cmpInstruction(GCONTXT * context);
u32int cmnInstruction(GCONTXT * context);
// mov operations
u32int movInstruction(GCONTXT * context);
u32int mvnInstruction(GCONTXT * context);
// shift register operations
u32int lslInstruction(GCONTXT * context);
u32int lsrInstruction(GCONTXT * context);
u32int asrInstruction(GCONTXT * context);
u32int rrxInstruction(GCONTXT * context);
u32int rorInstruction(GCONTXT * context);


#ifdef CONFIG_BLOCK_COPY

u32int* rsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* rscPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* subPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* sbcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* addPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* adcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* andPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* orrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* eorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* bicPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* tstPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* teqPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* cmpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* cmnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* movPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* mvnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* lslPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* lsrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* asrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* rrxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* rorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);

#endif /* CONFIG_BLOCK_COPY */

#endif
