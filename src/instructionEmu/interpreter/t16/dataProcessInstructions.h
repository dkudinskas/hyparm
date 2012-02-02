#ifndef __INSTRUCTION_EMU__INTERPRETER__T16__DATA_PROCESS_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__T16__DATA_PROCESS_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


/*
 * Arithmetic operations
 */

u32int t16AdcInstruction(GCONTXT *context, u32int instruction);
u32int t16AddInstruction(GCONTXT *context, u32int instruction);

u32int t16CmnInstruction(GCONTXT *context, u32int instruction);
u32int t16CmpInstruction(GCONTXT *context, u32int instruction);

u32int t16RsbInstruction(GCONTXT *context, u32int instruction);

u32int t16SubInstruction(GCONTXT *context, u32int instruction);
u32int t16SbcInstruction(GCONTXT *context, u32int instruction);

u32int t16MulInstruction(GCONTXT *context, u32int instruction);


/*
 * Bitwise operations
 */

u32int t16AndInstruction(GCONTXT *context, u32int instruction);
u32int t16EorInstruction(GCONTXT *context, u32int instruction);
u32int t16MvnInstruction(GCONTXT *context, u32int instruction);
u32int t16OrrInstruction(GCONTXT *context, u32int instruction);

u32int t16TstInstruction(GCONTXT *context, u32int instruction);

u32int t16BicInstruction(GCONTXT *context, u32int instruction);


/*
 * Move and shift register operations
 */

u32int t16MovInstruction(GCONTXT *context, u32int instruction);

u32int t16AsrInstruction(GCONTXT *context, u32int instruction);
u32int t16LslInstruction(GCONTXT *context, u32int instruction);
u32int t16LsrInstruction(GCONTXT *context, u32int instruction);
u32int t16RorInstruction(GCONTXT *context, u32int instruction);

#endif
