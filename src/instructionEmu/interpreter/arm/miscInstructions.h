#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for ARM instruction trace: #define ARM_INSTR_TRACE


/*
 * Control instructions
 */

u32int armBkptInstruction(GCONTXT *context, u32int instruction);

u32int armCpsInstruction(GCONTXT *context, u32int instruction);

u32int armDmbInstruction(GCONTXT *context, u32int instruction);
u32int armDsbInstruction(GCONTXT *context, u32int instruction);
u32int armIsbInstruction(GCONTXT *context, u32int instruction);

u32int armMrsInstruction(GCONTXT *context, u32int instruction);
u32int armMsrInstruction(GCONTXT *context, u32int instruction);

u32int armRfeInstruction(GCONTXT *context, u32int instruction);

u32int armSetendInstruction(GCONTXT *context, u32int instruction);

u32int armSmcInstruction(GCONTXT *context, u32int instruction);

u32int armSrsInstruction(GCONTXT *context, u32int instruction);


/*
 * Memory hints
 */

u32int armPldInstruction(GCONTXT *context, u32int instruction);
u32int armPliInstruction(GCONTXT *context, u32int instruction);


/*
 * NOP hints
 */

u32int armDbgInstruction(GCONTXT *context, u32int instruction);

u32int armSevInstruction(GCONTXT *context, u32int instruction);

u32int armWfeInstruction(GCONTXT *context, u32int instruction);
u32int armWfiInstruction(GCONTXT *context, u32int instruction);

u32int armYieldInstruction(GCONTXT *context, u32int instruction);


/*
 * Others
 */

u32int armClzInstruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_INSTRUCTIONS_H__ */
