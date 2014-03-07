#include "common/debug.h"
#include "common/stdlib.h"

#include "guestManager/guestContext.h"

#include "memoryManager/pageTableInfo.h"


void addPageTableInfo(GCONTXT *context, pageTableEntry* entry, u32int virtual, u32int physical, u32int mapped, bool host)
{
  DEBUG(MM_PAGE_TABLES, "addPageTableInfo: entry %#.8x @ %p, PA %#.8x VA %#.8x, mapped %#.8x host %x" EOL,
        *(u32int *)entry, entry, physical, virtual, mapped, host);

  ptInfo *newEntry = (ptInfo *)calloc(1, sizeof(ptInfo));
  DEBUG(MM_PAGE_TABLES, "addPageTableInfo: new entry @ %p" EOL, newEntry);
  newEntry->firstLevelEntry = entry;
  newEntry->physAddr = physical;
  newEntry->virtAddr = virtual;
  newEntry->host = host;
  newEntry->mappedMegabyte = mapped;
  newEntry->nextEntry = 0;


  ptInfo** headPtr;
  if (!context->virtAddrEnabled)
  {
    // virtual addressing is not enabled yet. this metadata has a special place
    headPtr = &context->pageTables->hptInfo;
  }
  else
  {
    headPtr = (host) ? &context->pageTables->sptInfo : &context->pageTables->gptInfo;
  }

  if (*headPtr == NULL)
  {
    // first entry. just set head to newEntry;
    *headPtr = newEntry;
  }
  else
  {
    // loop to the end of the list
    ptInfo* head = *headPtr;
    while (head->nextEntry != NULL)
    {
      head = head->nextEntry;
    }
    head->nextEntry = newEntry;
  }
  dumpPageTableInfo(context);
  // done.
}


ptInfo *getPageTableInfo(GCONTXT *context, pageTableEntry *firstLevelEntry)
{
  DEBUG(MM_PAGE_TABLES, "getPageTableInfo: first level entry ptr %p = %#.8x" EOL, firstLevelEntry,
        *(u32int *)firstLevelEntry);

  // hpt first
  ptInfo *head = context->pageTables->hptInfo;
  while (head != NULL)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
      DEBUG(MM_PAGE_TABLES, "getPageTableInfo: found spt; entry %p = %#.8x" EOL,
            head->firstLevelEntry, *(u32int *)head->firstLevelEntry);
      return head;
    }
    head = head->nextEntry;
  }

  // spt first
  head = context->pageTables->sptInfo;
  while (head != NULL)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
      DEBUG(MM_PAGE_TABLES, "getPageTableInfo: found spt; entry %p = %#.8x" EOL,
            head->firstLevelEntry, *(u32int *)head->firstLevelEntry);
      return head;
    }
    head = head->nextEntry;
  }

  // its not spt, gpt then?
  head = context->pageTables->gptInfo;
  while (head != NULL)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
      DEBUG(MM_PAGE_TABLES, "getPageTableInfo: found gpt; entry %p = %#.8x" EOL,
            head->firstLevelEntry, *(u32int *)head->firstLevelEntry);
      return head;
    }
    head = head->nextEntry;
  }

  // not found, return 0
  return 0;
}


void removePageTableInfo(GCONTXT *context, pageTableEntry* firstLevelEntry, bool host)
{
  DEBUG(MM_PAGE_TABLES, "removePageTableInfo: first level entry ptr %p = %#.8x" EOL,
        firstLevelEntry, *(u32int *)firstLevelEntry);

  ptInfo *head = (host) ? context->pageTables->sptInfo : context->pageTables->gptInfo;
  ptInfo *prev = NULL;

  while (head != NULL)
  {
    if (head->firstLevelEntry == firstLevelEntry)
    {
      DEBUG(MM_PAGE_TABLES, "removePageTableInfo: found entry %p = %#.8x" EOL,
            head->firstLevelEntry, *(u32int *)head->firstLevelEntry);
      ptInfo* tmp = head;
      head = head->nextEntry;
      if (host)
      {
        free((void *)tmp->virtAddr);
      }

      if (prev == 0)
      {
        // first entry in list.
        if (host)
        {
          context->pageTables->sptInfo = head;
        }
        else
        {
          context->pageTables->gptInfo = head;
        }
      }
      else
      {
        // not the first entry in list
        prev->nextEntry = head;
      }
      free(tmp);
      return;
    }
    prev = head;
    head = head->nextEntry;
  }
}


void dumpPageTableInfo(GCONTXT *context)
{
#if CONFIG_DEBUG_MM_PAGE_TABLES
  printf("dumpPageTableInfo:" EOL);

  // spt first
  ptInfo* head = context->pageTables->sptInfo;
  printf("sptInfo:" EOL);
  while (head != 0)
  {
    printf("%p: ptEntry %p; PA %#.8x VA %#.8x host %x" EOL, head, head->firstLevelEntry, head->physAddr, head->virtAddr, head->host);
    head = head->nextEntry;
  }

  // gpt then
  head = context->pageTables->gptInfo;
  printf("gptInfo:" EOL);
  while (head != 0)
  {
    printf("%p: ptEntry %p; PA %#.8x VA %#.8x host %x" EOL, head, head->firstLevelEntry, head->physAddr, head->virtAddr, head->host);
    head = head->nextEntry;
  }
#endif
}


void invalidatePageTableInfo(GCONTXT *context)
{
  DEBUG(MM_PAGE_TABLES, "invalidatePageTableInfo:" EOL);
  // spt first
  while (context->pageTables->sptInfo != NULL)
  {
    free((void *)context->pageTables->sptInfo->virtAddr);

    ptInfo* tempPtr = context->pageTables->sptInfo;
    context->pageTables->sptInfo = context->pageTables->sptInfo->nextEntry;
    free(tempPtr);
  }

  // gpt then
  while (context->pageTables->gptInfo != NULL)
  {
    ptInfo *tempPtr = context->pageTables->gptInfo;
    context->pageTables->gptInfo = context->pageTables->gptInfo->nextEntry;
    free(tempPtr);
  }

  DEBUG(MM_PAGE_TABLES, "invalidatePageTableInfo: ...done" EOL);
}
