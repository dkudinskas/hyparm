#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"


u32int armAluImmInstruction(GCONTXT *context, Instruction instr);
u32int armAluRegInstruction(GCONTXT *context, Instruction instr);

/*
 * Arithmetic operations
 */
u32int armCmnInstruction(GCONTXT *context, Instruction instr);
u32int armCmpInstruction(GCONTXT *context, Instruction instr);


/*
 * Bitwise operations
 */
u32int armMvnInstruction(GCONTXT *context, Instruction instr);
u32int armOrrInstruction(GCONTXT *context, Instruction instr);

u32int armTeqInstruction(GCONTXT *context, Instruction instr);
u32int armTstInstruction(GCONTXT *context, Instruction instr);

u32int armBicInstruction(GCONTXT *context, Instruction instr);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__ */
