#ifndef __INSTRUCTION_EMU__INTERPRETER__T16__STORE_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T16__STORE_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int t16StrInstruction(GCONTXT *context, u32int instruction);
u32int t16StrSpInstruction(GCONTXT *context, u32int instruction);
u32int t16StrbInstruction(GCONTXT *context, u32int instruction);
u32int t16StrhInstruction(GCONTXT *context, u32int instruction);

u32int t16PushInstruction(GCONTXT *context, u32int instruction);

#endif
