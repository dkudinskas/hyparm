#ifndef CONFIG_BLOCK_COPY
# include "instructionEmu/decoder/tableSearch.c"
#else
# include "instructionEmu/decoder/tableSearchBlockCopy.c"
#endif
