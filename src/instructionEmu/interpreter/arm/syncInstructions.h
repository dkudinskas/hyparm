#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__SYNC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__SYNC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"


u32int armClrexInstruction(GCONTXT *context, Instruction instr);

u32int armLdrexInstruction(GCONTXT *context, Instruction instr);
u32int armLdrexbInstruction(GCONTXT *context, Instruction instr);
u32int armLdrexhInstruction(GCONTXT *context, Instruction instr);
u32int armLdrexdInstruction(GCONTXT *context, Instruction instr);

u32int armStrexInstruction(GCONTXT *context, Instruction instr);
u32int armStrexbInstruction(GCONTXT *context, Instruction instr);
u32int armStrexhInstruction(GCONTXT *context, Instruction instr);
u32int armStrexdInstruction(GCONTXT *context, Instruction instr);

u32int armSwpInstruction(GCONTXT *context, Instruction instr);

#endif

