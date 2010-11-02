#ifndef __COPROC_INSTRUCTIONS_H_
#define __COPROC_INSTRUCTIONS_H_

#include "types.h"
#include "guestContext.h"

// uncomment me for coprocessor instruction trace: #define COPROC_INSTR_TRACE

u32int mcrrInstruction(GCONTXT * context);

u32int mrrcInstruction(GCONTXT * context);

u32int cdpInstruction(GCONTXT * context);

u32int mrcInstruction(GCONTXT * context);

u32int mcrInstruction(GCONTXT * context);

u32int stcInstruction(GCONTXT * context);

u32int ldcInstruction(GCONTXT * context);

/* V6 coprocessor instructions.  */
u32int mrrc2Instruction(GCONTXT * context);

u32int mcrr2Instruction(GCONTXT * context);

/* V5 coprocessor instructions.  */
u32int ldc2Instruction(GCONTXT * context);

u32int stc2Instruction(GCONTXT * context);

u32int cdp2Instruction(GCONTXT * context);

u32int mcr2Instruction(GCONTXT * context);

u32int mrc2Instruction(GCONTXT * context);

#endif
