#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armStrInstruction(GCONTXT *context, u32int instruction);
u32int armStrbInstruction(GCONTXT *context, u32int instruction);
u32int armStrhInstruction(GCONTXT *context, u32int instruction);
u32int armStrdInstruction(GCONTXT *context, u32int instruction);

u32int armStrtInstruction(GCONTXT *context, u32int instruction);
u32int armStrhtInstruction(GCONTXT *context, u32int instruction);
u32int armStrbtInstruction(GCONTXT *context, u32int instruction);

u32int armStmInstruction(GCONTXT *context, u32int instruction);

#endif
