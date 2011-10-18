#ifndef __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__
#define __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


// uncomment me for ARM instruction trace: #define DATA_PROC_TRACE


/*
 * Arithmetic operations
 */

u32int armAdcInstruction(GCONTXT *context, u32int instruction);
u32int armAddInstruction(GCONTXT *context, u32int instruction);
u32int armAdrInstruction(GCONTXT *context, u32int instruction);

u32int armCmnInstruction(GCONTXT *context, u32int instruction);
u32int armCmpInstruction(GCONTXT *context, u32int instruction);

u32int armRsbInstruction(GCONTXT *context, u32int instruction);
u32int armRscInstruction(GCONTXT *context, u32int instruction);

u32int armSubInstruction(GCONTXT *context, u32int instruction);
u32int armSbcInstruction(GCONTXT *context, u32int instruction);


/*
 * Bitwise operations
 */

u32int armAndInstruction(GCONTXT *context, u32int instruction);
u32int armEorInstruction(GCONTXT *context, u32int instruction);
u32int armMvnInstruction(GCONTXT *context, u32int instruction);
u32int armOrrInstruction(GCONTXT *context, u32int instruction);

u32int armTeqInstruction(GCONTXT *context, u32int instruction);
u32int armTstInstruction(GCONTXT *context, u32int instruction);

u32int armBicInstruction(GCONTXT *context, u32int instruction);


/*
 * Move and shift register operations
 */

u32int armMovInstruction(GCONTXT *context, u32int instruction);
u32int armMovwInstruction(GCONTXT *context, u32int instruction);
u32int armMovtInstruction(GCONTXT *context, u32int instruction);

u32int armAsrInstruction(GCONTXT *context, u32int instruction);
u32int armLslInstruction(GCONTXT *context, u32int instruction);
u32int armLsrInstruction(GCONTXT *context, u32int instruction);
u32int armRorInstruction(GCONTXT *context, u32int instruction);
u32int armRrxInstruction(GCONTXT *context, u32int instruction);

#endif /* __INSTRUCTION_EMU__INTERPRETER__ARM__DATA_PROCESS_INSTRUCTIONS_H__ */
