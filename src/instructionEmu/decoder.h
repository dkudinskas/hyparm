#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


typedef u32int (*instructionHandler)(GCONTXT * context);

typedef u32int* (*PCHandler)(GCONTXT * context, u32int * instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);


instructionHandler decodeArmInstruction(GCONTXT *context, u32int instruction);


#ifdef CONFIG_THUMB2

instructionHandler decodeThumbInstruction(GCONTXT *context, u32int instruction);

#endif /* CONFIG_THUMB2 */


#ifdef CONFIG_DECODER_TABLE_SEARCH
# include "instructionEmu/tableSearchDecoder.h"
#else
# ifdef CONFIG_DECODER_AUTO
#  include "instructionEmu/autoDecoder.h"
# else
#  error Decoder must be set!
# endif
#endif

#endif
