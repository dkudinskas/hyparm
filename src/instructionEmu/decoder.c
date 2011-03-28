#ifdef CONFIG_DECODER_TABLE_SEARCH
# ifdef CONFIG_BLOCK_COPY
#  include "instructionEmu/tableSearchBlockCopyDecoder.c"
# else
#  include "instructionEmu/tableSearchDecoder.c"
# endif
#else
# ifdef CONFIG_DECODER_AUTO
#  include "instructionEmu/autoDecoder.c"
# else
#  error Decoder must be set!
# endif
#endif
