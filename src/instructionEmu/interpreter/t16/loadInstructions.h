#ifndef __INSTRUCTION_EMU__INTERPRETER__T16__LOAD_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T16__LOAD_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int t16LdrInstruction(GCONTXT *context, u32int instruction);
u32int t16LdrbInstruction(GCONTXT *context, u32int instruction);
u32int t16LdrhImmediateInstruction(GCONTXT *context, u32int instruction);

u32int t16LdmInstruction(GCONTXT *context, u32int instruction);

#endif
