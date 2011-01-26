#ifndef __MEMORY_MANAGEMENT_UNIT_H__
#define __MEMORY_MANAGEMENT_UNIT_H__
#include "types.h"
#include "assert.h" //COMPILE_TIME_ASSERT
#include "pageTable.h"

struct abort_dfsr
{
  u16int fs3_0:4; //0-3 FS[3:0]
  u16int domain:4; //4-7
  u16int:2; //8-9 zero bits!
  u16int fs4:1;//10
  u16int WnR:1;//11
  u16int ExT:1;//12
  u32int:19;//12-31 more zero bits
};
typedef struct abort_dfsr DFSR;

struct abort_ifsr
{
  u16int fs3_0:4; //0-3 FS[3:0]
  u16int:6; //4-9 zero bits
  u16int fs4:1;//10
  u16int:1;//11 zero bit
  u16int ExT:1;//12
  u32int:19;//13-31 more zero bits
};
typedef struct abort_ifsr IFSR;

enum abort_fault_status
{
  alignment = 0b0001,
  debug,
  accessflag_section,
  icache_maint,
  translation_section,
  accessflag_page,
  translation_page,
  sync_external,
  unknown_abort_fault_status_0,
  domain_section,
  domain_page,
  trans_table_walk_1st,
  perm_section,
  sync_external_2nds,
  perm_page = 0b1111,
};

//access is a two bit field 00 = no access, 01=client, 10=reserved, 11=manager
enum enum_access_type
{
  no_access = 0b00,
  client,
  reserved,
  manager
};
typedef enum enum_access_type access_type;

void mmuInit(void);
void mmuInsertPt0(descriptor* addr);
void mmuInsertPt1(descriptor* addr);
descriptor* mmuGetPt0(void);
void mmuEnableVirtAddr(void);
void mmuDisableVirtAddr(void);
bool isMmuEnabled(void);

void clearCache(void);
void clearTLB(void);
void clearTLBbyMVA(u32int address);
void dataBarrier(void);
void setTTBCR(u32int value);
void setDomain(u8int domain, access_type access);

u32int getDFAR(void);
DFSR getDFSR(void);
u32int getIFAR(void);
IFSR getIFSR(void);

void printDataAbort(void); //gets & prints the dfsr & dfar
void printPrefetchAbort(void); //gets & prints the ifsr & ifar
#endif