HYPARM_SRCS_C-y += instructionEmu/interpreter/common.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/internals.c

HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/branchInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/coprocInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/dataProcessInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/loadInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/miscInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/miscMediaInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/multiplyInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/parallelAddSubInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/saturatingAddSubInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/storeInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/syncInstructions.c


HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/blockCopy.c

HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/branchPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/coprocPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/dataProcessPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/loadPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/miscPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/miscMediaPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/multiplyPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/parallelAddSubPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/saturatingAddSubPCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/storePCInstructions.c
HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += instructionEmu/interpreter/arm/syncPCInstructions.c


HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t16/branchInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t16/dataProcessInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t16/loadInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t16/miscInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t16/storeInstructions.c

HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t32/branchInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t32/dataProcessInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t32/loadInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t32/miscInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t32/multiplyInstructions.c
HYPARM_SRCS_C-$(CONFIG_THUMB2) += instructionEmu/interpreter/t32/storeInstructions.c
