#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


/*
 * Checks whether an instruction word of a Thumb instruction is a Thumb-32 instruction.
 */
#define TXX_IS_T32(instructionWord)  (instructionWord & 0xFFFF0000)


instructionHandler decodeArmInstruction(u32int instruction);


#ifdef CONFIG_THUMB2

instructionHandler decodeThumbInstruction(u32int instruction);

#endif /* CONFIG_THUMB2 */

#endif
