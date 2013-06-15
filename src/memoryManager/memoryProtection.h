#ifndef __MEMORY_MANAGER__MEMORY_PROTECTION_H__
#define __MEMORY_MANAGER__MEMORY_PROTECTION_H__

#include "common/types.h"

#include "guestManager/guestContext.h"

#include "memoryManager/pageTable.h" // for AccessType enum


// uncomment me for memory protection debug: #define MEM_PROT_DBG

#define DACR_NO_ACCESS   0
#define DACR_CLIENT      1
#define DACR_RESERVED    2
#define DACR_MANAGER     3

//access is a two bit field 00 = no access, 01=client, 10=reserved, 11=manager
enum enum_access_type
{
  no_access = 0b00,
  client,
  reserved,
  manager
};
typedef enum enum_access_type access_type;


typedef union
{
  struct domainAccessControlRegister
  {
    u32int dom0  : 2;
    u32int dom1  : 2;
    u32int dom2  : 2;
    u32int dom3  : 2;
    u32int dom4  : 2;
    u32int dom5  : 2;
    u32int dom6  : 2;
    u32int dom7  : 2;
    u32int dom8  : 2;
    u32int dom9  : 2;
    u32int dom10 : 2;
    u32int dom11 : 2;
    u32int dom12 : 2;
    u32int dom13 : 2;
    u32int dom14 : 2;
    u32int dom15 : 2;
  } fields;
  u32int value;
} DACR;

void guestWriteProtect(GCONTXT *gc, u32int startAddress, u32int endAddress);

void writeProtectRange(GCONTXT *gc, simpleEntry* pageTable, u32int start, u32int end);

// returns true if data abort to be delivered to guest
bool shouldDataAbort(GCONTXT *context, bool privAccess, bool isWrite, u32int address);

// returns true if prefetch abort to be delivered to guest
bool shouldPrefetchAbort(GCONTXT *context, bool privAccess, u32int address);

#endif
