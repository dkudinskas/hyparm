#ifndef __INSTRUCTION_EMU__INTERPRETER__T32__MULTIPLY_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T32__MULTIPLY_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


/*
 * Word multiply and multiply accumulate
 */

u32int t32MlaInstruction(GCONTXT *context, u32int instruction);
u32int t32MlsInstruction(GCONTXT *context, u32int instruction);
u32int t32MulInstruction(GCONTXT *context, u32int instruction);

u32int t32SmladInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlsdInstruction(GCONTXT *context, u32int instruction);

u32int t32SmmlaInstruction(GCONTXT *context, u32int instruction);
u32int t32SmmlsInstruction(GCONTXT *context, u32int instruction);
u32int t32SmmulInstruction(GCONTXT *context, u32int instruction);

u32int t32SmuadInstruction(GCONTXT *context, u32int instruction);
u32int t32SmusdInstruction(GCONTXT *context, u32int instruction);


/*
 * Halfword multiply and multiply accumulate
 */

u32int t32SmlabbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlabtInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlatbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlattInstruction(GCONTXT *context, u32int instruction);

u32int t32SmlawbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlawtInstruction(GCONTXT *context, u32int instruction);

u32int t32SmulbbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmulbtInstruction(GCONTXT *context, u32int instruction);
u32int t32SmultbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmulttInstruction(GCONTXT *context, u32int instruction);

u32int t32SmulwbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmulwtInstruction(GCONTXT *context, u32int instruction);


/*
 * Long multiply and multiply accumulate
 */

u32int t32SmlalInstruction(GCONTXT *context, u32int instruction);
u32int t32SmullInstruction(GCONTXT *context, u32int instruction);

u32int t32SmlalbbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlalbtInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlaltbInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlalttInstruction(GCONTXT *context, u32int instruction);

u32int t32SmlaldInstruction(GCONTXT *context, u32int instruction);
u32int t32SmlsldInstruction(GCONTXT *context, u32int instruction);

u32int t32UmaalInstruction(GCONTXT *context, u32int instruction);
u32int t32UmlalInstruction(GCONTXT *context, u32int instruction);
u32int t32UmullInstruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__MULTIPLY_INSTRUCTIONS_H__ */
