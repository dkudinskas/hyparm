#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_MEDIA_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_MEDIA_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


/*
 * Packing, unpacking, saturation and reversal
 */

u32int armPkhbtInstruction(GCONTXT *context, u32int instruction);
u32int armPkhtbInstruction(GCONTXT *context, u32int instruction);

u32int armRbitInstruction(GCONTXT *context, u32int instruction);

u32int armRevInstruction(GCONTXT *context, u32int instruction);
u32int armRev16Instruction(GCONTXT *context, u32int instruction);
u32int armRevshInstruction(GCONTXT *context, u32int instruction);

u32int armSelInstruction(GCONTXT *context, u32int instruction);

u32int armSsatInstruction(GCONTXT *context, u32int instruction);
u32int armSsat16Instruction(GCONTXT *context, u32int instruction);

u32int armSxtabInstruction(GCONTXT *context, u32int instruction);
u32int armSxtab16Instruction(GCONTXT *context, u32int instruction);
u32int armSxtahInstruction(GCONTXT *context, u32int instruction);

u32int armSxtbInstruction(GCONTXT *context, u32int instruction);
u32int armSxtb16Instruction(GCONTXT *context, u32int instruction);
u32int armSxthInstruction(GCONTXT *context, u32int instruction);

u32int armUsatInstruction(GCONTXT *context, u32int instruction);
u32int armUsat16Instruction(GCONTXT *context, u32int instruction);

u32int armUxtabInstruction(GCONTXT *context, u32int instruction);
u32int armUxtab16Instruction(GCONTXT *context, u32int instruction);
u32int armUxtahInstruction(GCONTXT *context, u32int instruction);

u32int armUxtbInstruction(GCONTXT *context, u32int instruction);
u32int armUxtb16Instruction(GCONTXT *context, u32int instruction);
u32int armUxthInstruction(GCONTXT *context, u32int instruction);


/*
 * Uncategorized
 */

u32int armBfcInstruction(GCONTXT *context, u32int instruction);
u32int armBfiInstruction(GCONTXT *context, u32int instruction);

u32int armSbfxInstruction(GCONTXT *context, u32int instruction);
u32int armUbfxInstruction(GCONTXT *context, u32int instruction);

u32int armUsad8Instruction(GCONTXT *context, u32int instruction);
u32int armUsada8Instruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__MISC_MEDIA_INSTRUCTIONS_H__ */
