#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"


u32int armAluImmInstruction(GCONTXT *context, u32int instruction);
u32int armAluRegInstruction(GCONTXT *context, u32int instruction);

/*
 * Arithmetic operations
 */
u32int armCmnInstruction(GCONTXT *context, u32int instruction);
u32int armCmpInstruction(GCONTXT *context, u32int instruction);


/*
 * Bitwise operations
 */
u32int armMvnInstruction(GCONTXT *context, u32int instruction);
u32int armOrrInstruction(GCONTXT *context, u32int instruction);

u32int armTeqInstruction(GCONTXT *context, u32int instruction);
u32int armTstInstruction(GCONTXT *context, u32int instruction);

u32int armBicInstruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__ */
