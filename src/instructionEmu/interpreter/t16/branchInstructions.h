#ifndef __INSTRUCTION_EMU__INTERPRETER__T16__BRANCH_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T16__BRANCH_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int t16BImmediate8Instruction(GCONTXT *context, u32int instruction);
u32int t16BImmediate11Instruction(GCONTXT *context, u32int instruction);

u32int t16BlxRegisterInstruction(GCONTXT *context, u32int instruction);

u32int t16BxInstruction(GCONTXT *context, u32int instruction);

#endif
