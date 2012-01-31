#ifdef CONFIG_BLOCK_COPY
# include "instructionEmu/tableSearchBlockCopyDecoder.c"
#else
# include "instructionEmu/tableSearchDecoder.c"
#endif
