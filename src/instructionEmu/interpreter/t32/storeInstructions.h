#ifndef __INSTRUCTION_EMU__INTERPRETER__T32__STORE_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T32__STORE_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int t32StrbInstruction(GCONTXT *context, u32int instruction);
u32int t32StrhImmediateInstruction(GCONTXT *context, u32int instruction);
u32int t32StrhRegisterInstruction(GCONTXT *context, u32int instruction);
u32int t32StrdImmediateInstruction(GCONTXT *context, u32int instruction);

u32int t32StrhtInstruction(GCONTXT *context, u32int instruction);

#endif
