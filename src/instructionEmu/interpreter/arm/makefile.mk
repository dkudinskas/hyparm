HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/loadInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/storeInstructions.c

HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/loadPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/storePCInstructions.c