HYPARM_SRCS_C-y += instructionEmu/interpreter/common.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/internals.c

HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/branchInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/coprocInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/aluInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/loadInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/miscInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/storeInstructions.c
HYPARM_SRCS_C-y += instructionEmu/interpreter/arm/syncInstructions.c


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
