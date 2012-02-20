#ifndef __INSTRUCTION_EMU__INTERPRETER__T32__BRANCH_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T32__BRANCH_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

u32int t32MRSInstruction(GCONTXT *context, u32int instruction);

u32int t32BImmediate17Instruction(GCONTXT *context, u32int instruction);
u32int t32BImmediate21Instruction(GCONTXT *context, u32int instruction);

u32int t32BlInstruction(GCONTXT *context, u32int instruction);
u32int t32BlxImmediateInstruction(GCONTXT *context, u32int instruction);

#endif
