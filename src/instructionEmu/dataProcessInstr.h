#ifndef __INSTRUCTION_EMU__DATA_PROCESS_INSTR__
#define __INSTRUCTION_EMU__DATA_PROCESS_INSTR__

#include "common/types.h"
#include "hardware/serial.h"
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
void invalidDataProcTrap(char * msg, GCONTXT * gc);
u32int arithLogicOp(GCONTXT * context, OPTYPE opType, char * instrString);
// Arithmetic operations

#ifdef CONFIG_BLOCK_COPY
u32int* rsbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int rsbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* rscPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int rscInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* subPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int subInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* sbcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int sbcInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* addPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int addInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* adcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int adcInstruction(GCONTXT * context);
// bitwise logic operations

#ifdef CONFIG_BLOCK_COPY
u32int* andPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int andInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* orrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int orrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* eorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int eorInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* bicPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int bicInstruction(GCONTXT * context);
// bitwise compare operations

#ifdef CONFIG_BLOCK_COPY
u32int* tstPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int tstInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* teqPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int teqInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* cmpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int cmpInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* cmnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int cmnInstruction(GCONTXT * context);
// mov operations

#ifdef CONFIG_BLOCK_COPY
u32int* movPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int movInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mvnPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mvnInstruction(GCONTXT * context);
// shift register operations

#ifdef CONFIG_BLOCK_COPY
u32int* lslPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int lslInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* lsrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int lsrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* asrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int asrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* rrxPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int rrxInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* rorPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int rorInstruction(GCONTXT * context);
#endif
