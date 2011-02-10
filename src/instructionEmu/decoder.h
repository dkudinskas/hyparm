#ifndef __DECODER_H__
#define __DECODER_H__

#include "guestContext.h"
#include "types.h"

typedef u32int (*instructionHandler)(GCONTXT * context);

#ifdef CONFIG_DECODER_TABLE_SEARCH
# include "tableSearchDecoder.h"
#else
# ifdef CONFIG_DECODER_AUTO
#  include "autoDecoder.h"
# else
#  error Decoder must be set!
# endif
#endif

#endif
