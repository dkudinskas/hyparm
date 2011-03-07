#ifndef __COPROC_INSTRUCTIONS_H_
#define __COPROC_INSTRUCTIONS_H_
#include "types.h"
#include "guestContext.h"
// uncomment me for coprocessor instruction trace: #define COPROC_INSTR_TRACE

u32int mcrrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mcrrInstruction(GCONTXT * context);

u32int mrrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mrrcInstruction(GCONTXT * context);

u32int cdpPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int cdpInstruction(GCONTXT * context);

u32int mrcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mrcInstruction(GCONTXT * context);

u32int mcrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mcrInstruction(GCONTXT * context);

u32int stcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int stcInstruction(GCONTXT * context);

u32int ldcPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldcInstruction(GCONTXT * context);
/* V6 coprocessor instructions.  */

u32int mrrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mrrc2Instruction(GCONTXT * context);

u32int mcrr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mcrr2Instruction(GCONTXT * context);
/* V5 coprocessor instructions.  */

u32int ldc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int ldc2Instruction(GCONTXT * context);

u32int stc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int stc2Instruction(GCONTXT * context);

u32int cdp2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int cdp2Instruction(GCONTXT * context);

u32int mcr2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mcr2Instruction(GCONTXT * context);

u32int mrc2PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);
u32int mrc2Instruction(GCONTXT * context);
#endif
