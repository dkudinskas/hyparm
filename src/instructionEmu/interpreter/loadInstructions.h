#ifndef __INSTRUCTION_EMU__EMULATOR__LOAD_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__EMULATOR__LOAD_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for LOAD/STORE instruction trace: #define DATA_MOVE_TRACE


u32int armLdrInstruction(GCONTXT *context, u32int instruction);
u32int armLdrbInstruction(GCONTXT *context, u32int instruction);
u32int armLdrhInstruction(GCONTXT *context, u32int instruction);
u32int armLdrdInstruction(GCONTXT *context, u32int instruction);

u32int armLdrhtInstruction(GCONTXT *context, u32int instruction);

u32int armLdrexInstruction(GCONTXT *context, u32int instr);
u32int armLdrexbInstruction(GCONTXT *context, u32int instr);
u32int armLdrexhInstruction(GCONTXT *context, u32int instr);
u32int armLdrexdInstruction(GCONTXT *context, u32int instr);

u32int armLdmInstruction(GCONTXT *context, u32int instruction);


u32int t16LdrInstruction(GCONTXT *context, u32int instruction);
u32int t16LdrbInstruction(GCONTXT *context, u32int instruction);
u32int t16LdrhImmediateInstruction(GCONTXT *context, u32int instruction);

u32int t16LdmInstruction(GCONTXT *context, u32int instruction);


u32int t32LdrbInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrhImmediateInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrhLiteralInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrhRegisterInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrdInstruction(GCONTXT *context, u32int instruction);

u32int t32LdrshImmediate8Instruction(GCONTXT *context, u32int instruction);
u32int t32LdrshImmediate12Instruction(GCONTXT *context, u32int instruction);
u32int t32LdrshLiteralInstruction(GCONTXT *context, u32int instruction);
u32int t32LdrshRegisterInstruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__EMULATOR__LOAD_INSTRUCTIONS_H__ */
