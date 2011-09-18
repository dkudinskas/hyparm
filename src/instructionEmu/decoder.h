#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


typedef u32int (*instructionHandler)(GCONTXT *context);


instructionHandler decodeArmInstruction(u32int instruction);


#ifdef CONFIG_THUMB2

instructionHandler decodeThumbInstruction(u32int instruction);

#endif /* CONFIG_THUMB2 */

#endif
