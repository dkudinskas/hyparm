#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__SATURATING_ADD_SUB_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__SATURATING_ADD_SUB_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armQaddInstruction(GCONTXT *context, u32int instruction);
u32int armQsubInstruction(GCONTXT *context, u32int instruction);

u32int armQdaddInstruction(GCONTXT *context, u32int instruction);
u32int armQdsubInstruction(GCONTXT *context, u32int instruction);

#endif
