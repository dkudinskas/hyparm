#ifndef __DATA_MOVE_INSTR__
#define __DATA_MOVE_INSTR__
#include "types.h"
#include "serial.h"
#include "guestContext.h"
#include "globalMemoryMapper.h"
// uncomment me for LOAD/STORE instruction trace: #define DATA_MOVE_TRACE
void invalidDataProcTrap(char * msg, GCONTXT * gc);

u32int* strPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strInstruction(GCONTXT * context);

u32int* strbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strbInstruction(GCONTXT * context);

u32int* strhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strhInstruction(GCONTXT * context);

u32int* stmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int stmInstruction(GCONTXT * context);

u32int* strdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strdInstruction(GCONTXT * context);

u32int* strexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strexInstruction(GCONTXT * context);

u32int* strexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strexbInstruction(GCONTXT * context);

u32int* strexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strexdInstruction(GCONTXT * context);

u32int* strexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strexhInstruction(GCONTXT * context);

u32int* strhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int strhtInstruction(GCONTXT * context);

u32int* ldrhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrhInstruction(GCONTXT * context);

u32int* ldrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrInstruction(GCONTXT * context);

u32int* ldrdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrdInstruction(GCONTXT * context);

u32int* ldrbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrbInstruction(GCONTXT * context);

u32int* popLdrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int popLdrInstruction(GCONTXT * context);

u32int* popLdmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int popLdmInstruction(GCONTXT * context);

u32int* ldmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldmInstruction(GCONTXT * context);

u32int* ldrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrexInstruction(GCONTXT * context);

u32int* ldrexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrexbInstruction(GCONTXT * context);

u32int* ldrexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrexdInstruction(GCONTXT * context);

u32int* ldrexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrexhInstruction(GCONTXT * context);

u32int* ldrhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldrhtInstruction(GCONTXT * context);
#endif
