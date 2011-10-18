#ifndef __INSTRUCTION_EMU__INTERPRETER__T32__LOAD_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T32__LOAD_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int t32LdrbInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrhImmediateInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrhLiteralInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrhRegisterInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrdInstruction(GCONTXT *context, u32int instruction);

u32int t32LdrshImmediate8Instruction(GCONTXT *context, u32int instruction);
u32int t32LdrshImmediate12Instruction(GCONTXT *context, u32int instruction);
u32int t32LdrshLiteralInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrshRegisterInstruction(GCONTXT *context, u32int instruction);

#endif
