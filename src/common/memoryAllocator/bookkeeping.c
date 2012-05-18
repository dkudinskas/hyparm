#include "common/debug.h"
#include "common/stdlib.h"


typedef struct entry
{
  const char *file;
  const char *line;
  const char *function;
  const void *pointer;
  u32int size;
  struct entry *prevEntry;
  struct entry *nextEntry;
} entry;


static void addEntry(const char *file, const char *line, const char *function, const void *ptr, u32int size);
static entry *findEntry(void *ptr);
static void removeEntry(entry *e);


// use a linked list for now (this is a debugging thing anyway)
// inited to zero from startup.s
static entry *allocatorLog;
static entry *allocatorLogLast;


void dumpAllocatorBookkeeping(void);


static void addEntry(const char *file, const char *line, const char *function, const void *ptr, u32int size)
{
  entry *e = uncheckedMalloc(sizeof(struct entry));
  e->file = file;
  e->line = line;
  e->function = function;
  e->pointer = ptr;
  e->size = size;
  e->nextEntry = NULL;
  if (allocatorLog == NULL)
  {
    allocatorLog = e;
    e->prevEntry = NULL;
  }
  else
  {
    e->prevEntry = allocatorLogLast;
    allocatorLogLast->nextEntry = e;
  }
  allocatorLogLast = e;
}

void checkedFree(const char *file, const char *line, const char *function, void *ptr)
{
  DEBUG(MEMORY_ALLOCATOR_BOOKKEEPING, "%s:%s: in %s: free(%p)" EOL, file, line, function, ptr);
  /*
   * Don't bother with NULL pointers, but warn because we shouldn't be calling free on them anyway.
   */
  if (ptr == NULL)
  {
    printf("%s:%s: in %s: free on NULL pointer" EOL, file, line, function);
    return;
  }
  /*
   * Try to find the entry in the log. If we don't find one, the pointer is invalid!
   */
  entry *e = findEntry(ptr);
  if (e == NULL)
  {
    printf("%s:%s: in %s: free on invalid pointer %p" EOL, file, line, function, ptr);
    DIE_NOW(NULL, "malloc/free mismatch");
  }
  /*
   * We have an entry, remove it from the log and free it
   */
  removeEntry(e);
  uncheckedFree(ptr);
}

void *checkedMalloc(const char *file, const char *line, const char *function, u32int size)
{
  DEBUG(MEMORY_ALLOCATOR_BOOKKEEPING, "%s:%s: in %s: malloc(%#.8x)" EOL, file, line, function,
        size);
  void *ptr = uncheckedMalloc(size);
  addEntry(file, line, function, ptr, size);
  DEBUG(MEMORY_ALLOCATOR_BOOKKEEPING, "%s:%s: in %s: malloc(%#.8x) = %p" EOL, file, line, function,
        size, ptr);
  return ptr;
}

void *checkedMemalign(const char *file, const char *line, const char *function, u32int alignment,
                      u32int size)
{
  DEBUG(MEMORY_ALLOCATOR_BOOKKEEPING, "%s:%s: in %s: memalign(%#.8x, %#.8x)" EOL, file, line,
        function, alignment, size);
  void *ptr = uncheckedMemalign(alignment, size);
  addEntry(file, line, function, ptr, size);
  DEBUG(MEMORY_ALLOCATOR_BOOKKEEPING, "%s:%s: in %s: memalign(%#.8x, %#.8x) = %p" EOL, file, line,
        function, alignment, size, ptr);
  return ptr;
}

void *checkedRealloc(const char *file, const char *line, const char *function, void *ptr,
                     u32int size)
{
  DEBUG(MEMORY_ALLOCATOR_BOOKKEEPING, "%s:%s: in %s: realloc(%p, %#.8x)" EOL, file, line, function,
        ptr, size);
  /*
   * With NULL pointers, realloc behaves like malloc. Warn because we should avoid using realloc.
   * With non-NULL pointers, see where they come from, and remove their log entry.
   */
  if (ptr == NULL)
  {
    printf("%s:%s: in %s: realloc on NULL pointer" EOL, file, line, function);
  }
  else
  {
    entry *e = findEntry(ptr);
    if (e == NULL)
    {
      printf("%s:%s: in %s: realloc on invalid pointer %p" EOL, file, line, function, ptr);
      DIE_NOW(NULL, "malloc/realloc mismatch");
    }
    removeEntry(e);
    free(e);
  }
  /*
   * Invoke realloc and read entry to log.
   */
  void *newPtr = uncheckedRealloc(ptr, size);
  DEBUG(MEMORY_ALLOCATOR_BOOKKEEPING, "%s:%s: in %s: realloc(%p, %#.8x) = %p" EOL, file, line,
        function, ptr, size, newPtr);
  if (newPtr == NULL)
  {
    if (size != 0)
    {
      printf("%s:%s: in %s: realloc(%p, %#.8x) failed" EOL, file, line, function, ptr, size);
    }
  }
  else
  {
    addEntry(file, line, function, ptr, size);
  }
  return newPtr;
}

void dumpAllocatorBookkeeping()
{
  printf("Allocated memory:" EOL);
  for (entry *e = allocatorLog; e != NULL; e = e->nextEntry)
  {
    printf("%s:%s: in %s: allocated %p size %#.8x" EOL, e->file, e->line, e->function, e->pointer,
           e->size);
  }
}

static entry *findEntry(void *ptr)
{
  /*
   * Do a reverse lookup in the log...
   */
  entry *e = allocatorLogLast;
  while (e != NULL)
  {
    if (e->pointer == ptr)
    {
      return e;
    }
    e = e->prevEntry;
  }
  return NULL;
}

static void removeEntry(entry *e)
{
  /*
   * If there is a previous entry, set its next pointer.
   * If not, this entry might be the first entry!
   */
  if (e->prevEntry == NULL)
  {
    allocatorLog = e->nextEntry;
  }
  else
  {
    e->prevEntry->nextEntry = e->nextEntry;
  }
  /*
   * If there is no next entry, the previous entry is now last.
   * Otherwise, update the reverse pointer of the next entry.
   */
  if (e->nextEntry == NULL)
  {
    allocatorLogLast = e->prevEntry;
  }
  else
  {
    e->nextEntry->prevEntry = e->prevEntry;
  }
  /*
   * Don't forget to deallocate!
   */
  uncheckedFree(e);
}
