#ifndef __MEMORY_MANAGER__MMU_H__
#define __MEMORY_MANAGER__MMU_H__

#include "common/types.h"

#include "memoryManager/pageTable.h"


//uncomment to enable debug #define MMU_DBG

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

enum DataAbortFaultStatus
{
  dfsAlignmentFault = 0b00001,
  dfsDebugEvent = 0b00010,
  dfsAccessFlagSection = 0b00011,
  dfsIcacheMaintenance = 0b00100,
  dfsTranslationSection = 0b00101,
  dfsAccessFlagPage = 0b00110,
  dfsTranslationPage = 0b00111,
  dfsSyncExternalAbt = 0b01000,
  dfsDomainSection = 0b01001,
  dfsDomainPage = 0b01011,
  dfsTranslationTableWalkLvl1SyncExtAbt = 0b01100,
  dfsPermissionSection = 0b01101,
  dfsTranslationTableWalkLvl2SyncExtAbt = 0b01110,
  dfsPermissionPage = 0b01111,
  dfsImpDepLockdown = 0b10100,
  dfsAsyncExternalAbt = 0b10110,
  dfsMemAccessAsyncParityErr = 0b11000,
  dfsMemAccessAsyncParityERr2 = 0b11001,
  dfsImpDepCoprocessorAbort = 0b11010,
  dfsTranslationTableWalkLvl1SyncParityErr = 0b11100,
  dfsTranslationTableWalkLvl2SyncParityErr = 0b11110,
};

enum InstructionAbortFaultStatus
{
  ifsDebugEvent = 0b00010,
  ifsAccessFlagFaultSection = 0b00011,
  ifsTranslationFaultSection = 0b00101,
  ifsAccessFlagFaultPage = 0b00110,
  ifsTranslationFaultPage = 0b00111,
  ifsSynchronousExternalAbort = 0b01000,
  ifsDomainFaultSection = 0b01001,
  ifsDomainFaultPage = 0b01011 ,
  ifsTranslationTableTalk1stLvlSynchExtAbt = 0b01100,
  ifsPermissionFaultSection = 0b01101,
  ifsTranslationTableWalk2ndLvllSynchExtAbt = 0b01110,
  ifsPermissionFaultPage = 0b01111,
  ifsImpDepLockdown = 0b10100,
  ifsMemoryAccessSynchParityError = 0b11001,
  ifsImpDepCoprocessorAbort = 0b11010,
  ifsTranslationTableWalk1stLvlSynchParityError = 0b11100,
  ifsTranslationTableWalk2ndLvlSynchParityError = 0b11110,
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
void mmuSetTTBCR(u32int value);
void mmuSetTTBR0(simpleEntry* addr);
void mmuSetTTBR1(simpleEntry* addr);
simpleEntry* mmuGetTTBR0(void);

void mmuEnableVirtAddr(void);
void mmuDisableVirtAddr(void);
bool isMmuEnabled(void);


void mmuClearInstructionCache(void);
void mmuClearDataCache(void);
void mmuClearTLB(void);
void mmuClearTLBbyMVA(u32int address);
void mmuDataBarrier(void);
void mmuSetDomain(u8int domain, access_type access);
void mmuSetTexRemap(bool enable);

void mmuInvalidateIcacheByMVA(u32int mva);
void mmuCleanDcacheByMVA(u32int mva);

u32int getDFAR(void);
DFSR getDFSR(void);
u32int getIFAR(void);
IFSR getIFSR(void);

void printDataAbort(void); //gets & prints the dfsr & dfar
void printPrefetchAbort(void); //gets & prints the ifsr & ifar
#endif
