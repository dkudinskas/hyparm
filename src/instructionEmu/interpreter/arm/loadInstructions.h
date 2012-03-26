#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armLdrInstruction(GCONTXT *context, u32int instruction);
u32int armLdrbInstruction(GCONTXT *context, u32int instruction);
u32int armLdrhInstruction(GCONTXT *context, u32int instruction);
u32int armLdrdInstruction(GCONTXT *context, u32int instruction);

u32int armLdrtInstruction(GCONTXT *context, u32int instruction);
u32int armLdrhtInstruction(GCONTXT *context, u32int instruction);
u32int armLdrbtInstruction(GCONTXT *context, u32int instruction);

u32int armLdmInstruction(GCONTXT *context, u32int instruction);

#endif
