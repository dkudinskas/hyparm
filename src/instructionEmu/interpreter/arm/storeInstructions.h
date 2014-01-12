#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__STORE_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"

// byte stores
u32int armStrbImmInstruction(GCONTXT *context, Instruction instr);
u32int armStrbRegInstruction(GCONTXT *context, Instruction instr);
u32int armStrbtImmInstruction(GCONTXT *context, Instruction instr);
u32int armStrbtRegInstruction(GCONTXT *context, Instruction instr);
// half-word stores
u32int armStrhImmInstruction(GCONTXT *context, Instruction instr);
u32int armStrhRegInstruction(GCONTXT *context, Instruction instr);
u32int armStrhtImmInstruction(GCONTXT *context, Instruction instr);
u32int armStrhtRegInstruction(GCONTXT *context, Instruction instr);
// word stores
u32int armStrImmInstruction(GCONTXT* context, Instruction instr);
u32int armStrRegInstruction(GCONTXT *context, Instruction instr);
u32int armStrtImmInstruction(GCONTXT *context, Instruction instr);
u32int armStrtRegInstruction(GCONTXT *context, Instruction instr);
// dual word stores
u32int armStrdImmInstruction(GCONTXT* context, Instruction instr);
u32int armStrdRegInstruction(GCONTXT* context, Instruction instr);
// multi stores
u32int armStmInstruction(GCONTXT *context, Instruction instr);
u32int armStmUserInstruction(GCONTXT *context, Instruction instr);


#endif
