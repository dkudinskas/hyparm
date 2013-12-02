#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"

// byte loads
u32int armLdrbImmInstruction(GCONTXT *context, u32int instruction);
u32int armLdrbRegInstruction(GCONTXT *context, u32int instruction);
u32int armLdrbtImmInstruction(GCONTXT *context, u32int instruction);
u32int armLdrbtRegInstruction(GCONTXT *context, u32int instruction);
// halfword loads
u32int armLdrhImmInstruction(GCONTXT *context, u32int instruction);
u32int armLdrhRegInstruction(GCONTXT *context, u32int instruction);
u32int armLdrhtImmInstruction(GCONTXT *context, u32int instruction);
u32int armLdrhtRegInstruction(GCONTXT *context, u32int instruction);
// word loads
u32int armLdrImmInstruction(GCONTXT *context, u32int instruction);
u32int armLdrRegInstruction(GCONTXT *context, u32int instruction);
u32int armLdrtImmInstruction(GCONTXT *context, u32int instruction);
u32int armLdrtRegInstruction(GCONTXT *context, u32int instruction);
// dual loads
u32int armLdrdImmInstruction(GCONTXT* context, u32int instruction);
u32int armLdrdRegInstruction(GCONTXT* context, u32int instruction);

u32int armLdmInstruction(GCONTXT *context, u32int instruction);


#endif
