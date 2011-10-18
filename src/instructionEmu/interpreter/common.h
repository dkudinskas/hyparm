#ifndef __INSTRUCTION_EMU__INTERPRETER__COMMON_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__COMMON_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int nopInstruction(GCONTXT *context, u32int instruction);

u32int svcInstruction(GCONTXT *context, u32int instruction);

u32int undefinedInstruction(GCONTXT *context, u32int instruction);

#endif
