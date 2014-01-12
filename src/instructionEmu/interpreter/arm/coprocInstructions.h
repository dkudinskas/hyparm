#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__COPROC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__COPROC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"

u32int armCdpInstruction(GCONTXT *context, Instruction instr);
u32int armCdp2Instruction(GCONTXT *context, Instruction instr);

u32int armLdcInstruction(GCONTXT *context, Instruction instr);
u32int armLdc2Instruction(GCONTXT *context, Instruction instr);

u32int armMcrInstruction(GCONTXT *context, Instruction instr);
u32int armMcr2Instruction(GCONTXT *context, Instruction instr);
u32int armMcrrInstruction(GCONTXT *context, Instruction instr);
u32int armMcrr2Instruction(GCONTXT *context, Instruction instr);

u32int armMrcInstruction(GCONTXT *context, Instruction instr);
u32int armMrc2Instruction(GCONTXT *context, Instruction instr);
u32int armMrrcInstruction(GCONTXT *context, Instruction instr);
u32int armMrrc2Instruction(GCONTXT *context, Instruction instr);

u32int armStcInstruction(GCONTXT *context, Instruction instr);
u32int armStc2Instruction(GCONTXT *context, Instruction instr);

#endif
