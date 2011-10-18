#ifndef __INSTRUCTION_EMU__INTERPRETER__T16__MISC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T16__MISC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int t16ItInstruction(GCONTXT *context, u32int instruction);

u32int t16UxtbInstruction(GCONTXT *context, u32int instruction);
u32int t16UxthInstruction(GCONTXT *context, u32int instruction);

#endif
