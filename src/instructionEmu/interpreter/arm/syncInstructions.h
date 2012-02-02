#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__SYNC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__SYNC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armClrexInstruction(GCONTXT *context, u32int instruction);

u32int armLdrexInstruction(GCONTXT *context, u32int instruction);
u32int armLdrexbInstruction(GCONTXT *context, u32int instruction);
u32int armLdrexhInstruction(GCONTXT *context, u32int instruction);
u32int armLdrexdInstruction(GCONTXT *context, u32int instruction);

u32int armStrexInstruction(GCONTXT *context, u32int instruction);
u32int armStrexbInstruction(GCONTXT *context, u32int instruction);
u32int armStrexhInstruction(GCONTXT *context, u32int instruction);
u32int armStrexdInstruction(GCONTXT *context, u32int instruction);

u32int armSwpInstruction(GCONTXT *context, u32int instruction);

#endif

