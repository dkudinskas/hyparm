#ifdef CONFIG_DECODER_TABLE_SEARCH
# include "tableSearchDecoder.c"
#else
# ifdef CONFIG_DECODER_AUTO
#  include "autoDecoder.c"
# else
#  error Decoder must be set!
# endif
#endif
