#include "common/debug.h"
#include "common/stdlib.h"

#include "memoryManager/pageTableInfo.h"


void addPageTableInfo(pageTableEntry* entry, u32int virtual, u32int physical, u32int mapped, bool host)
{
  DEBUG(MM_PAGE_TABLES, "addPageTableInfo: entry %#.8x @ %p, PA %#.8x VA %#.8x, mapped %#.8x host %x" EOL,
        *(u32int *)entry, entry, physical, virtual, mapped, host);
  GCONTXT *context = getGuestContext();

  ptInfo *newEntry = (ptInfo *)malloc(sizeof(ptInfo));
  DEBUG(MM_PAGE_TABLES, "addPageTableInfo: new entry @ %p" EOL, newEntry);
  newEntry->firstLevelEntry = entry;
  newEntry->physAddr = physical;
  newEntry->virtAddr = virtual;
  newEntry->host = host;
  newEntry->mappedMegabyte = mapped;
  newEntry->nextEntry = 0;

  ptInfo *head = (host) ? context->pageTables->sptInfo : context->pageTables->gptInfo;
  if (head == NULL)
  {
    // first entry
    if (host)
    {
      DEBUG(MM_PAGE_TABLES, "addPageTableInfo: first entry, sptInfo now %p" EOL, newEntry);
      context->pageTables->sptInfo = newEntry;
    }
    else
    {
      DEBUG(MM_PAGE_TABLES, "addPageTableInfo: first entry, gptInfo now %p" EOL, newEntry);
      context->pageTables->gptInfo = newEntry;
    }
  }
  else
  {
    // loop to the end of the list
    DEBUG(MM_PAGE_TABLES, "addPageTableInfo: not the first entry. head %p" EOL, head);
    while (head->nextEntry != NULL)
    {
      head = head->nextEntry;
    }
    head->nextEntry = newEntry;
  }
  dumpPageTableInfo();
  // done.
}


ptInfo *getPageTableInfo(pageTableEntry *firstLevelEntry)
{
  DEBUG(MM_PAGE_TABLES, "getPageTableInfo: first level entry ptr %p = %#.8x" EOL, firstLevelEntry,
        *(u32int *)firstLevelEntry);
  GCONTXT* context = getGuestContext();

  // spt first
  ptInfo *head = context->pageTables->sptInfo;
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


void removePageTableInfo(pageTableEntry* firstLevelEntry, bool host)
{
  DEBUG(MM_PAGE_TABLES, "removePageTableInfo: first level entry ptr %p = %#.8x" EOL,
        firstLevelEntry, *(u32int *)firstLevelEntry);
  GCONTXT *context = getGuestContext();

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
      free((void *)tmp);
      return;
    }
    prev = head;
    head = head->nextEntry;
  }
}


void dumpPageTableInfo()
{
#if CONFIG_DEBUG_MM_PAGE_TABLES
  printf("dumpPageTableInfo:" EOL);

  GCONTXT* context = getGuestContext();
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


void invalidatePageTableInfo()
{
  DEBUG(MM_PAGE_TABLES, "invalidatePageTableInfo:" EOL);
  GCONTXT* context = getGuestContext();
  // spt first
  while (context->pageTables->sptInfo != NULL)
  {
    // zero fields
    context->pageTables->sptInfo->firstLevelEntry->type = FAULT;
    context->pageTables->sptInfo->firstLevelEntry = NULL;
    context->pageTables->sptInfo->physAddr = 0;
    free((void*)context->pageTables->sptInfo->virtAddr);
    context->pageTables->sptInfo->virtAddr = 0;

    ptInfo* tempPtr = context->pageTables->sptInfo;
    context->pageTables->sptInfo = context->pageTables->sptInfo->nextEntry;
    free((void*)tempPtr);
  }
  context->pageTables->sptInfo = NULL;

  // gpt then
  while (context->pageTables->gptInfo != NULL)
  {
    // zero fields
    context->pageTables->gptInfo->firstLevelEntry->type = FAULT;
    context->pageTables->gptInfo->firstLevelEntry = NULL;
    context->pageTables->gptInfo->physAddr = 0;
    context->pageTables->gptInfo->virtAddr = 0;
    context->pageTables->gptInfo->host = FALSE;

    ptInfo *tempPtr = context->pageTables->gptInfo;
    context->pageTables->gptInfo = context->pageTables->gptInfo->nextEntry;
    free((void *)tempPtr);
  }
  context->pageTables->gptInfo = NULL;

  DEBUG(MM_PAGE_TABLES, "invalidatePageTableInfo: ...done" EOL);
}
