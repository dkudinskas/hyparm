#ifndef __INSTRUCTION_EMU__EMULATOR__BRANCH_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__EMULATOR__BRANCH_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armBInstruction(GCONTXT *context, u32int instruction);

u32int armBlxImmediateInstruction(GCONTXT *context, u32int instruction);
u32int armBlxRegisterInstruction(GCONTXT *context, u32int instruction);

u32int armBxInstruction(GCONTXT *context, u32int instruction);

u32int armBxjInstruction(GCONTXT *context, u32int instruction);


u32int t16BImmediate8Instruction(GCONTXT *context, u32int instruction);
u32int t16BImmediate11Instruction(GCONTXT *context, u32int instruction);

u32int t16BlxRegisterInstruction(GCONTXT *context, u32int instruction);

u32int t16BxInstruction(GCONTXT *context, u32int instruction);


u32int t32BImmediate17Instruction(GCONTXT *context, u32int instruction);
u32int t32BImmediate21Instruction(GCONTXT *context, u32int instruction);

u32int t32BlInstruction(GCONTXT *context, u32int instruction);
u32int t32BlxImmediateInstruction(GCONTXT *context, u32int instruction);

#endif
