#ifndef __INSTRUCTION_EMU__CONTEXT_SWITCH_COUNTERS_H__
#define __INSTRUCTION_EMU__CONTEXT_SWITCH_COUNTERS_H__

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"


#ifdef CONFIG_CONTEXT_SWITCH_COUNTERS
void countBranch(GCONTXT* context, Instruction instr);
void countBL(GCONTXT* context, Instruction instr);
void countBLXreg(GCONTXT* context, Instruction instr);
void countBX(GCONTXT* context, Instruction instr);

void registerSvc(GCONTXT *context, InstructionHandler handler);
#else
#define countBranch(context, instr);
#define countBL(context, instr);
#define countBLXreg(context, instr);
#define countBX(context, instr);

#define registerSvc(context, handler);
#endif


#endif
