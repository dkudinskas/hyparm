HYPARM_SRCS_C-$(CONFIG_MEMORY_ALLOCATOR_BOOKKEEPING) += common/memoryAllocator/bookkeeping.c

HYPARM_SRCS_C-$(CONFIG_MEMORY_ALLOCATOR_NAIVE) += common/memoryAllocator/naive.c
HYPARM_SRCS_C-$(CONFIG_MEMORY_ALLOCATOR_TLSF) += common/memoryAllocator/tlsf.c
HYPARM_SRCS_C-$(CONFIG_MEMORY_ALLOCATOR_DLMALLOC) += common/memoryAllocator/dlmalloc.c

