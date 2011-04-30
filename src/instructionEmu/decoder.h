#ifndef __INSTRUCTION_EMU__DECODER_H__
#define __INSTRUCTION_EMU__DECODER_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


typedef u32int (*instructionHandler)(GCONTXT * context);

#define T_BIT			0x20   // 5th bit of CPSR

// 3 different thumb-32 bit encodings.
#define THUMB32			0xF800
#define	THUMB32_1		0xE800
#define THUMB32_2		0xF000
#define THUMB32_3		0xF800

// markers to be used by endOfHalfBlock variable
#define LHALF			0x01
#define HHALF			0x02
#define WTHUMB32		0x03
#define WHTHUMB32		0x04

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
