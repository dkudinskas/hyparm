#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/decoder/arm/structs.h"

/*
 * Control instructions
 */

u32int armBkptInstruction(GCONTXT *context, Instruction instr);

u32int armCpsInstruction(GCONTXT *context, Instruction instr);

u32int armMrsInstruction(GCONTXT *context, Instruction instr);
u32int armMsrImmInstruction(GCONTXT *context, Instruction instr);
u32int armMsrRegInstruction(GCONTXT *context, Instruction instr);

u32int armRfeInstruction(GCONTXT *context, Instruction instr);

u32int armSetendInstruction(GCONTXT *context, Instruction instr);

u32int armSmcInstruction(GCONTXT *context, Instruction instr);

u32int armSrsInstruction(GCONTXT *context, Instruction instr);



/*
 * NOP hints
 */

u32int armDbgInstruction(GCONTXT *context, Instruction instr);

u32int armSevInstruction(GCONTXT *context, Instruction instr);

u32int armWfeInstruction(GCONTXT *context, Instruction instr);
u32int armWfiInstruction(GCONTXT *context, Instruction instr);

u32int armYieldInstruction(GCONTXT *context, Instruction instr);


/*
 * Others
 */

u32int armClzInstruction(GCONTXT *context, Instruction instr);

u32int svcInstruction(GCONTXT *context, Instruction instr);

u32int undefinedInstruction(GCONTXT *context, Instruction instr);


#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_INSTRUCTIONS_H__ */
