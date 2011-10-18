#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__BRANCH_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__BRANCH_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armBInstruction(GCONTXT *context, u32int instruction);

u32int armBlxImmediateInstruction(GCONTXT *context, u32int instruction);
u32int armBlxRegisterInstruction(GCONTXT *context, u32int instruction);

u32int armBxInstruction(GCONTXT *context, u32int instruction);

u32int armBxjInstruction(GCONTXT *context, u32int instruction);

#endif
