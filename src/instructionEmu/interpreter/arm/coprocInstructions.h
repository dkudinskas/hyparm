#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__COPROC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__COPROC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


u32int armCdpInstruction(GCONTXT *context, u32int instruction);
u32int armCdp2Instruction(GCONTXT *context, u32int instruction);

u32int armLdcInstruction(GCONTXT *context, u32int instruction);
u32int armLdc2Instruction(GCONTXT *context, u32int instruction);

u32int armMcrInstruction(GCONTXT *context, u32int instruction);
u32int armMcr2Instruction(GCONTXT *context, u32int instruction);
u32int armMcrrInstruction(GCONTXT *context, u32int instruction);
u32int armMcrr2Instruction(GCONTXT *context, u32int instruction);

u32int armMrcInstruction(GCONTXT *context, u32int instruction);
u32int armMrc2Instruction(GCONTXT *context, u32int instruction);
u32int armMrrcInstruction(GCONTXT *context, u32int instruction);
u32int armMrrc2Instruction(GCONTXT *context, u32int instruction);

u32int armStcInstruction(GCONTXT *context, u32int instruction);
u32int armStc2Instruction(GCONTXT *context, u32int instruction);

#endif
