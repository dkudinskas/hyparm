HYPARM_SRCS_C-y += instructionEmu/commonInstrFunctions.c
HYPARM_SRCS_C-y += instructionEmu/coprocInstructions.c
HYPARM_SRCS_C-y += instructionEmu/dataProcessInstr.c
HYPARM_SRCS_C-y += instructionEmu/miscInstructions.c
HYPARM_SRCS_C-y += instructionEmu/scanner.c

HYPARM_SRCS_C-$(CONFIG_DECODER_AUTO) += instructionEmu/autoDecoder.c

HYPARM_SRCS_C-$(CONFIG_DECODER_TABLE_SEARCH) += instructionEmu/tableSearchDecoder.c
