#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__MULTIPLY_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__MULTIPLY_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


/*
 * Word multiply and multiply accumulate (miscellaneous instructions)
 */

u32int armMlaInstruction(GCONTXT *context, u32int instruction);
u32int armMlsInstruction(GCONTXT *context, u32int instruction);
u32int armMulInstruction(GCONTXT *context, u32int instruction);

u32int armSmlalInstruction(GCONTXT *context, u32int instruction);
u32int armSmullInstruction(GCONTXT *context, u32int instruction);

u32int armUmaalInstruction(GCONTXT *context, u32int instruction);

u32int armUmlalInstruction(GCONTXT *context, u32int instruction);
u32int armUmullInstruction(GCONTXT *context, u32int instruction);


/*
 * Halfword multiply and multiply accumulate (miscellaneous instructions)
 */

u32int armSmlabbInstruction(GCONTXT *context, u32int instruction);
u32int armSmlabtInstruction(GCONTXT *context, u32int instruction);
u32int armSmlatbInstruction(GCONTXT *context, u32int instruction);
u32int armSmlattInstruction(GCONTXT *context, u32int instruction);

u32int armSmlalbbInstruction(GCONTXT *context, u32int instruction);
u32int armSmlalbtInstruction(GCONTXT *context, u32int instruction);
u32int armSmlaltbInstruction(GCONTXT *context, u32int instruction);
u32int armSmlalttInstruction(GCONTXT *context, u32int instruction);

u32int armSmlawbInstruction(GCONTXT *context, u32int instruction);
u32int armSmlawtInstruction(GCONTXT *context, u32int instruction);

u32int armSmulbbInstruction(GCONTXT *context, u32int instruction);
u32int armSmulbtInstruction(GCONTXT *context, u32int instruction);
u32int armSmultbInstruction(GCONTXT *context, u32int instruction);
u32int armSmulttInstruction(GCONTXT *context, u32int instruction);

u32int armSmulwbInstruction(GCONTXT *context, u32int instruction);
u32int armSmulwtInstruction(GCONTXT *context, u32int instruction);


/*
 * Signed multiply (media instructions)
 */

u32int armSmladInstruction(GCONTXT *context, u32int instruction);
u32int armSmlsdInstruction(GCONTXT *context, u32int instruction);
u32int armSmuadInstruction(GCONTXT *context, u32int instruction);
u32int armSmusdInstruction(GCONTXT *context, u32int instruction);

u32int armSmlaldInstruction(GCONTXT *context, u32int instruction);
u32int armSmlsldInstruction(GCONTXT *context, u32int instruction);

u32int armSmmlaInstruction(GCONTXT *context, u32int instruction);
u32int armSmmlsInstruction(GCONTXT *context, u32int instruction);

u32int armSmmulInstruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__MULTIPLY_INSTRUCTIONS_H__ */
