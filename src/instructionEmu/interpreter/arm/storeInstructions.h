#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"

// byte stores
u32int armStrbImmInstruction(GCONTXT *context, u32int instruction);
u32int armStrbRegInstruction(GCONTXT *context, u32int instruction);
u32int armStrbtImmInstruction(GCONTXT *context, u32int instruction);
u32int armStrbtRegInstruction(GCONTXT *context, u32int instruction);
// half-word stores
u32int armStrhImmInstruction(GCONTXT *context, u32int instruction);
u32int armStrhRegInstruction(GCONTXT *context, u32int instruction);
u32int armStrhtImmInstruction(GCONTXT *context, u32int instruction);
u32int armStrhtRegInstruction(GCONTXT *context, u32int instruction);
// word stores
u32int armStrImmInstruction(GCONTXT* context, u32int instruction);
u32int armStrRegInstruction(GCONTXT *context, u32int instruction);
u32int armStrtImmInstruction(GCONTXT *context, u32int instruction);
u32int armStrtRegInstruction(GCONTXT *context, u32int instruction);
// dual word stores
u32int armStrdImmInstruction(GCONTXT* context, u32int instruction);
u32int armStrdRegInstruction(GCONTXT* context, u32int instruction);
// multi stores
u32int armStmInstruction(GCONTXT *context, u32int instruction);
u32int armStmUserInstruction(GCONTXT *context, u32int instruction);


#endif
