#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__BRANCH_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__BRANCH_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"

u32int armBInstruction(GCONTXT *context, Instruction instr);
u32int armBlInstruction(GCONTXT *context, Instruction instr);

u32int armBlxImmediateInstruction(GCONTXT *context, Instruction instr);
u32int armBlxRegisterInstruction(GCONTXT *context, Instruction instr);

u32int armBxInstruction(GCONTXT *context, Instruction instr);

u32int armBxjInstruction(GCONTXT *context, Instruction instr);

#endif
