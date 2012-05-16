HYPARM_SRCS_C-y += guestManager/guestContext.c
HYPARM_SRCS_C-y += guestManager/guestExceptions.c
HYPARM_SRCS_C-y += guestManager/scheduler.c
HYPARM_SRCS_C-y += guestManager/translationCache.c

HYPARM_SRCS_C-$(CONFIG_BLOCK_COPY) += guestManager/codeCacheAllocator.c
