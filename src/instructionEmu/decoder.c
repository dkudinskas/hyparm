#ifdef CONFIG_DECODER_TABLE_SEARCH
# include "instructionEmu/tableSearchDecoderWrapper.c"
#else
# ifdef CONFIG_DECODER_AUTO
#  include "instructionEmu/autoDecoder.c"
# else
#  error Decoder must be set!
# endif
#endif
