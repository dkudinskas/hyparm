#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


typedef enum
{
  IRC_SAFE = 0,
  IRC_REPLACE = 1
} instructionReplaceCode;


instructionReplaceCode decodeArmInstruction(u32int instruction, instructionHandler *handler);


#ifdef CONFIG_THUMB2

instructionReplaceCode decodeThumbInstruction(u32int instruction, instructionHandler *handler);

#endif /* CONFIG_THUMB2 */

#endif
