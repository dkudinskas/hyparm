#ifndef __MEMORY_MANAGER__MMU_H__
#define __MEMORY_MANAGER__MMU_H__

#include "common/compiler.h"
#include "common/stddef.h"
#include "common/types.h"

#include "memoryManager/pageTable.h"


typedef enum
{
  TTP_PRIVILEGED_READ,
  TTP_PRIVILEGED_WRITE,
  TTP_UNPRIVILEGED_READ,
  TTP_UNPRIVILEGED_WRITE
} TryTranslatePurpose;

struct dfsr
{
  unsigned fs3_0:4; //0-3 FS[3:0]
  unsigned domain:4; //4-7
  unsigned : 2; //8-9 zero bits!
  unsigned fs4:1;//10
  unsigned WnR:1;//11
  unsigned ExT:1;//12
  unsigned : 19;//12-31 more zero bits
};

struct ifsr
{
  unsigned fs3_0 : 4; //0-3 FS[3:0]
  unsigned : 6; //4-9 zero bits
  unsigned fs4 : 1;//10
  unsigned : 1;//11 zero bit
  unsigned ExT : 1;//12
  unsigned : 19;//13-31 more zero bits
};

struct physicalAddressRegisterFault
{
  unsigned fault : 1;
  unsigned faultStatus : 6;
  unsigned : 25;
};

struct physicalAddressRegisterSuccess
{
  unsigned fault : 1;
  unsigned superSection : 1;
  unsigned outerAttributes : 2;
  unsigned innerAttributes : 3;
  unsigned shareable : 1;
  unsigned : 1;
  unsigned nonSecure : 1;
  unsigned notOuterShareable : 1;
  unsigned : 1;
  unsigned physicalAddress_31_20 : 20;
};

struct systemControlRegister
{
  unsigned mmuEnable : 1;
  unsigned alignmentCheckingEnable : 1;
  unsigned cacheEnable : 1;
  unsigned : 7;
  unsigned swapEnable : 1;
  unsigned branchPredictionEnable : 1;
  unsigned instructionCacheEnable : 1;
  unsigned highVectors : 1;
  unsigned roundRobin : 1;
  unsigned : 2;
  unsigned hardwareAccessFlagEnable : 1;
  unsigned : 3;
  unsigned fiLowLatencyEnable : 1;
  unsigned : 2;
  unsigned interruptVectorsEnable : 1;
  unsigned exceptionEndianness : 1;
  unsigned : 1;
  unsigned nonMaskableFastInterrupts : 1;
  unsigned texRemapEnable : 1;
  unsigned accessFlagEnable : 1;
  unsigned thumbExceptionEnable : 1;
  unsigned : 1;
};

COMPILE_TIME_ASSERT((sizeof(struct dfsr) == sizeof(u32int)), _DFSR_struct_not_32bit);
COMPILE_TIME_ASSERT((sizeof(struct ifsr) == sizeof(u32int)), _IFSR_struct_not_32bit);
COMPILE_TIME_ASSERT((sizeof(struct physicalAddressRegisterFault) == sizeof(u32int)), _PAR_fault_struct_not_32bit);
COMPILE_TIME_ASSERT((sizeof(struct physicalAddressRegisterSuccess) == sizeof(u32int)), _PAR_success_struct_not_32bit);
COMPILE_TIME_ASSERT((sizeof(struct systemControlRegister) == sizeof(u32int)), _SCTRL_struct_not_32bit);

typedef struct dfsr DFSR;

typedef struct ifsr IFSR;

typedef union
{
  struct physicalAddressRegisterFault fault;
  struct physicalAddressRegisterSuccess success;
  u32int value;
} PhysicalAddressRegister;

typedef union
{
  struct systemControlRegister bits;
  u32int value;
} SystemControlRegister;


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


__macro__ bool isMmuEnabled(void);

void mmuInit(void);
void mmuSetTTBCR(u32int value);
void mmuSetTTBR0(simpleEntry* addr, u32int asid);
void mmuSetTTBR1(simpleEntry* addr, u32int asid);
simpleEntry* mmuGetTTBR0(void);

void mmuEnableVirtAddr(void);
void mmuDisableVirtAddr(void);

void mmuDisableAlignmentChecking(void);

void mmuInvIcacheToPOU(void);
void mmuInvIcacheByMVAtoPOU(u32int mva);

void mmuInstructionSync(void);

void mmuInvBranchPredictorArray(void);

void mmuCleanDcacheByMVAtoPOC(u32int mva);
void mmuCleanDcacheBySetWay(u32int mva);

void mmuCleanDCacheByMVAtoPOU(u32int mva);
void mmuCleanInvDCacheByMVAtoPOC(u32int mva);
void mmuCleanInvDCacheBySetWay(u32int setWay);

void mmuDataSyncBarrier(void);
void mmuDataMemoryBarrier(void);

void mmuClearInstructionCache(void);
void mmuClearDataCache(void);

void mmuInvalidateITLB(void);
void mmuInvalidateITLBbyMVA(u32int mva);
void mmuInvalidateITLBbyASID(u32int asid);

void mmuInvalidateDTLB(void);
void mmuInvalidateDTLBbyMVA(u32int mva);
void mmuInvalidateDTLBbyASID(u32int asid);

void mmuInvalidateUTLB(void);
void mmuInvalidateUTLBbyMVA(u32int mva);

void mmuClearTLBbyMVA(u32int address);
void mmuSetDomain(u8int domain, access_type access);
void mmuSetTexRemap(bool enable);
void mmuSetContextID(u32int asid);

void mmuSetExceptionVector(u32int vectorBase);

u32int getDFAR(void);
DFSR getDFSR(void);
u32int getIFAR(void);
IFSR getIFSR(void);

void mmuPageTableEdit(u32int entryAddr, u32int pageAddr);

__macro__ PhysicalAddressRegister mmuTryTranslate(u32int virtualAddress, TryTranslatePurpose purpose);

void printDataAbort(void) __cold__; //gets & prints the dfsr & dfar
void printPrefetchAbort(void) __cold__; //gets & prints the ifsr & ifar


__macro__ bool isMmuEnabled()
{
  SystemControlRegister sctrl;
  __asm__ __volatile__("MRC p15, 0, %0, c1, c0, 0":"=r"(sctrl));
  return sctrl.bits.mmuEnable;
}

/*
 * mmuTryTranslate
 * Attempts to translate a given virtual address to a physical address and only generates an abort
 * if the translation fails because an external abort occurred on a translation table walk request.
 */
__macro__ PhysicalAddressRegister mmuTryTranslate(u32int virtualAddress, TryTranslatePurpose purpose)
{
  PhysicalAddressRegister par;
  switch (purpose)
  {
    case TTP_PRIVILEGED_READ:
      __asm__ __volatile__("MCR p15, 0, %0, c7, c8, 0"::"r"(virtualAddress));
      break;
    case TTP_PRIVILEGED_WRITE:
      __asm__ __volatile__("MCR p15, 0, %0, c7, c8, 1"::"r"(virtualAddress));
      break;
    case TTP_UNPRIVILEGED_READ:
      __asm__ __volatile__("MCR p15, 0, %0, c7, c8, 2"::"r"(virtualAddress));
      break;
    case TTP_UNPRIVILEGED_WRITE:
    default:
      __asm__ __volatile__("MCR p15, 0, %0, c7, c8, 3"::"r"(virtualAddress));
      break;
  }
  __asm__ __volatile__("ISB; MRC p15, 0, %0, c7, c4, 0":"=r"(par));
  return par;
}


#endif
