#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


typedef u32int (*instructionHandler)(GCONTXT * context);

#define T_BIT			0x20

#define THUMB32			0xF8000000
#define	THUMB32_1		0x1D
#define THUMB32_2		0x1E
#define THUMB32_3		0x1F

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
