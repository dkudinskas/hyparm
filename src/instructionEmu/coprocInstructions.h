#ifndef __INSTRUCTION_EMU__COPROC_INSTRUCTIONS_H_
#define __INSTRUCTION_EMU__COPROC_INSTRUCTIONS_H_

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for coprocessor instruction trace: #define COPROC_INSTR_TRACE

#ifdef CONFIG_BLOCK_COPY
u32int* mcrrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mcrrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mrrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mrrcInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* cdpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int cdpInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mrcInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mcrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mcrInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* stcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int stcInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* ldcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldcInstruction(GCONTXT * context);
/* V6 coprocessor instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* mrrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mrrc2Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mcrr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mcrr2Instruction(GCONTXT * context);
/* V5 coprocessor instructions.  */

#ifdef CONFIG_BLOCK_COPY
u32int* ldc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int ldc2Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* stc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int stc2Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* cdp2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int cdp2Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mcr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mcr2Instruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* mrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif
u32int mrc2Instruction(GCONTXT * context);
#endif
