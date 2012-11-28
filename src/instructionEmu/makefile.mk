HYPARM_SRCS_C-y += instructionEmu/blockLinker.c
HYPARM_SRCS_C-y += instructionEmu/loadStoreDecode.c
HYPARM_SRCS_C-y += instructionEmu/scanner.c

HYPARM_SRCS_C-$(CONFIG_LOOP_DETECTOR) += instructionEmu/loopDetector.c
