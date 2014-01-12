#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__LOAD_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"

// byte loads
u32int armLdrbImmInstruction(GCONTXT *context, Instruction instr);
u32int armLdrbRegInstruction(GCONTXT *context, Instruction instr);
u32int armLdrbtImmInstruction(GCONTXT *context, Instruction instr);
u32int armLdrbtRegInstruction(GCONTXT *context, Instruction instr);
// halfword loads
u32int armLdrhImmInstruction(GCONTXT *context, Instruction instr);
u32int armLdrhRegInstruction(GCONTXT *context, Instruction instr);
u32int armLdrhtImmInstruction(GCONTXT *context, Instruction instr);
u32int armLdrhtRegInstruction(GCONTXT *context, Instruction instr);
// word loads
u32int armLdrImmInstruction(GCONTXT *context, Instruction instr);
u32int armLdrRegInstruction(GCONTXT *context, Instruction instr);
u32int armLdrtImmInstruction(GCONTXT *context, Instruction instr);
u32int armLdrtRegInstruction(GCONTXT *context, Instruction instr);
// dual loads
u32int armLdrdImmInstruction(GCONTXT* context, Instruction instr);
u32int armLdrdRegInstruction(GCONTXT* context, Instruction instr);
// multiple loads
u32int armLdmInstruction(GCONTXT *context, Instruction instr);
u32int armLdmUserInstruction(GCONTXT *context, Instruction instr);
u32int armLdmExcRetInstruction(GCONTXT *context, Instruction instr);


#endif
