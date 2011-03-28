#ifndef __INSTRUCTION_EMU__DATA_MOVE_INSTR__
#define __INSTRUCTION_EMU__DATA_MOVE_INSTR__

#include "common/types.h"

#include "memoryManager/globalMemoryMapper.h"

#include "guestManager/guestContext.h"


// uncomment me for LOAD/STORE instruction trace: #define DATA_MOVE_TRACE
void invalidDataProcTrap(char * msg, GCONTXT * gc);

#ifdef CONFIG_BLOCK_COPY
u32int* strPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strhInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* stmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int stmInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strdInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strexInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strexbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strexdInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strexhInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int strhtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrhInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrdInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* popLdrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int popLdrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* popLdmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int popLdmInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldmInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrexInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrexbInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrexdInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrexhInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldrhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldrhtInstruction(GCONTXT * context);
#endif
