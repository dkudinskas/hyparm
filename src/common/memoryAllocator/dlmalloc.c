#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#pragma GCC diagnostic ignored "-Wunused-function"


#include "common/assert.h"
#include "common/compiler.h"
#include "common/types.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/memoryAllocator/allocator.h"


COMPILE_TIME_ASSERT(sizeof(size_t) == sizeof(void *), "dlmalloc requires size_t to be pointer size");


// Useful to abort instead of assert when assert(..) has the potential to call malloc/new
#define ABORT_ON_ASSERT_FAILURE   0

#ifdef CONFIG_ASSERT_EXTENSIVE
// Extensive checking and assertions
#define DEBUG_DLMALLOC                     1
#else
#define DEBUG_DLMALLOC                     0
#endif

#ifdef CONFIG_DLMALLOC_FOOTERS
// Extra checking and dispatching through chunk footers
#define FOOTERS                   1
#ifdef CONFIG_ASSERT
#define CORRUPTION_ERROR_ACTION(m)  ASSERT(0, "memory allocator state is corrupt")
#define USAGE_ERROR_ACTION(m,p)     ASSERT(0, "memory allocator usage error")
#endif
#endif

#ifndef CONFIG_ASSERT
// Omit checks for usage errors and heap space overwrites
#define INSECURE                   1
#endif

// Required for POSIX functions but unused by hlibc
#define EINVAL                     -1
#define ENOMEM                     -1

#define DLMALLOC_EXPORT            static
#define HAVE_MORECORE              0
#define HAVE_MMAP                  0
#define HAVE_MREMAP                0
#define LACKS_ERRNO_H
#define LACKS_FCNTL_H
#define LACKS_SCHED_H
#define LACKS_STDLIB_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_MMAN_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_TYPES_H
#define LACKS_TIME_H
#define LACKS_UNISTD_H
#define MALLOC_FAILURE_ACTION
#define NO_MALLOC_STATS            1
#define USE_BUILTIN_FFS            1
#define USE_DL_PREFIX

#define MSPACES                    1
#define ONLY_MSPACES               1


// assert helper (for now)
#define assert(cond)  ASSERT(cond, "");


#include "dlmalloc/malloc.c"

static mspace staticPool;

void initialiseAllocator(u32int startAddress, u32int bytes)
//void initialiseAllocator(void *start, size_t bytes)
{
  staticPool = create_mspace_with_base((void*)startAddress, bytes, 0);
}

void free(void *ptr)
{
  mspace_free(staticPool, ptr);
}

void* malloc(size_t size)
{
  void *pointer = mspace_malloc(staticPool, size);
  return pointer;
}

void* memalign(size_t boundary, size_t size)
{
  void *pointer = mspace_memalign(staticPool, boundary, size);
  return pointer;
}

void *realloc(void *pointer, size_t size)
{
  pointer = mspace_realloc(staticPool, pointer, size);
  return pointer;
}
