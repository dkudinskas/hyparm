#include "common/assert.h"
#include "common/debug.h"

#include "memoryManager/mmu.h"

#include "cpuArch/armv7.h"
#include "cpuArch/cache.h"


static const char *const dataAbtFaultString[] =
{
  "INVALID ENTRY",
  "Alignment Fault",
  "Debug Event",
  "Access Flag - Section",
  "Instruction Cache Maintainance Fault",
  "Translation Fault - Section",
  "Access Flag - Page",
  "Translation Fault - Page",
  "Syncronous External Abort",
  "Domain Fault - Section",
  "INVALID ENTRY",
  "Domain Fault - Page",
  "Translation Table Walk - 1st Level",
  "Permission Fault - Section",
  "Syncronous External Abort - 2nd Level",
  "Permission Fault - Page",
};

static const char *const prefetchAbtFaultString[] =
{
  "INVALID ENTRY",                            //  0 = 0b00000,
  "INVALID ENTRY",                            //  1 = 0b00001,
  "Debug Event",                              //  2 = 0b00010,
  "Access Flag Fault: Section",               //  3 = 0b00011,
  "INVALID ENTRY",                            //  4 = 0b00100,
  "Translation Fault: Section",               //  5 = 0b00101,
  "Access Flag Fault: Page",                  //  6 = 0b00110,
  "Translation Fault: Page",                  //  7 = 0b00111,
  "Synchronous External Abort",               //  8 = 0b01000,
  "Domain Fault: Section",                    //  9 = 0b01001,
  "INVALID ENTRY",                            //  a = 0b01010,
  "Domain Fault: Page",                       //  b = 0b01011,
  "Translation Table Talk 1st Level sync abt",//  c = 0b01100,
  "Permission Fault: Section"                 //  d = 0b01101,
  "Translation Table Walk 2nd Level sync abt",//  e = 0b01110,
  "Permission Fault: Page",                   //  f = 0b01111,
  "INVALID ENTRY",                            // 10 = 0b10000,
  "INVALID ENTRY",                            // 11 = 0b10001,
  "INVALID ENTRY",                            // 12 = 0b10010,
  "INVALID ENTRY",                            // 13 = 0b10011,
  "IMPLEMENTATION DEFINED Lockdown",          // 14 = 0b10100,
  "INVALID ENTRY",                            // 15 = 0b10101,
  "INVALID ENTRY",                            // 16 = 0b10110,
  "INVALID ENTRY",                            // 17 = 0b10111,
  "INVALID ENTRY",                            // 18 = 0b11000,
  "Memory Access Synchronous Parity Error",   // 19 = 0b11001,
  "IMPLEMENTATION DEFINED Coprocessor Abort", // 1a = 0b11010,
  "INVALID ENTRY",                            // 1b = 0b11011,
  "Translation Table Walk 1st Level parity",  // 1c = 0b11100,
  "INVALID ENTRY",                            // 1d = 0b11101,
  "Translation Table Walk 2nd Level parity",  // 1e = 0b11110,
  "INVALID ENTRY",                            // 1f = 0b11111,
};


void mmuInit()
{
#ifdef MMU_DBG
  printf("MMU init" EOL);
#endif

  mmuDataMemoryBarrier();
  mmuInvalidateUTLB();
  mmuClearInstructionCache();
  mmuSetTTBR0(0, 0);
}

/**
 * set translation table base constrol register
 **/
void mmuSetTTBCR(u32int value)
{
#ifdef MMU_DBG
  printf("MMU: set TTBCR to %x" EOL, value);
#endif
  __asm__ __volatile__("mcr p15, 0, %0, c2, c0, 2"
  :
  :"r"(value)
     );
}

void mmuSetTTBR0(simpleEntry* addr, u32int asid)
{
#ifdef MMU_DBG
  printf("MMU: set TTBR0 to %08x" EOL, (u32int)addr);
#endif
  // set context id to reserved value
  mmuSetContextID(0);

  // instruction sync barrier
  mmuInstructionSync();
  
  __asm__ __volatile__("mcr p15, 0, %0, c2, c0, 0": :"r"(addr));

  // instruction sync barrier
  mmuInstructionSync();

  // set context id to new value
  mmuSetContextID(asid);

  // instruction sync barrier
  mmuInstructionSync();
}

void mmuSetTTBR1(simpleEntry* addr, u32int asid)
{
#ifdef MMU_DBG
  printf("MMU: set translation table base register 1 to %08x" EOL, (u32int)addr);
#endif
  // set context id to reserved value
  mmuSetContextID(0);

  // instruction sync barrier
  mmuInstructionSync();
  
  __asm__ __volatile__("mcr p15, 0, %0, c2, c0, 1": :"r"(addr));

  // instruction sync barrier
  mmuInstructionSync();

  // set context id to new value
  mmuSetContextID(asid);
}

simpleEntry* mmuGetTTBR0()
{
  u32int regVal = 0;
  //TODO: need to improve this to insert the correct bit masks
  __asm__ __volatile__("mrc p15, 0, %0, c2, c0, 0":"=r"(regVal));
#ifdef MMU_DBG
  printf("MMU: get translation table base register 0, val %08x" EOL, (u32int)regVal);
#endif
  return (simpleEntry*)regVal;
}


void mmuEnableVirtAddr()
{
  u32int tempReg;
  //This may need a bit of investigation/logic to ensure the bit masks we set are correct
  __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0\n\t"
               "ORR %0, %0, #5\n\t" //enable MMU & Caching
               "mcr p15, 0, %0, c1, c0, 0\n\t"
               "mcr p15, 0, %0, c7, c5, 4\n\t" //ISB
  :"=r"(tempReg)
  :
  : "memory"
     );
}

void mmuDisableVirtAddr()
{
  u32int tempReg = 0;
  //This may need a bit of investigation/logic to ensure the bit masks we set are correct
  __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0\n\t"
               "BIC %0, %0, #5\n\t" //enable MMU & Caching
               "mcr p15, 0, %0, c1, c0, 0\n\t"
  :"=r"(tempReg)
  :
  : "memory"
     );
}

bool isMmuEnabled()
{
  u32int tempReg = 0;
  //This may need a bit of investigation/logic to ensure the bit masks we set are correct
  __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0\n\t" :"=r"(tempReg));
  
  return (tempReg & 0x1) ? TRUE : FALSE;
         
}


/**
 * clear and invalidate host instruction cache
 **/
void mmuClearInstructionCache()
{
#ifdef MMU_DBG
  printf("mmuClearInstructionCache: clearing host iCache" EOL);
#endif
  asm ("mcr p15, 0, %0, c7, c5, 0": : "r"(0));
}


void mmuInvIcacheToPOU(void)
{
#ifdef MMU_DBG
  printf("mmuInvIcacheByMVAtoPOU: inv all iCaches to PoU" EOL);
#endif
  __asm__ __volatile__("mcr p15, 0, %0, c7, c5, 0": :"r"(0));
}


void mmuInvIcacheByMVAtoPOU(u32int mva)
{
#ifdef MMU_DBG
  printf("mmuInvIcacheByMVAtoPOU: inv iCache by MVA to PoU %08x" EOL, mva);
#endif

  __asm__ __volatile__("mcr p15, 0, %0, c7, c5, 1": :"r"(mva));
}


void mmuInstructionSync()
{
#ifdef MMU_DBG
  printf("mmuInstructionSync" EOL);
#endif
  asm ("mcr p15, 0, %0, c7, c5, 4": : "r"(0));
}


void mmuInvBranchPredictorArray()
{
#ifdef CPU_CORTEX_A8
#ifdef MMU_DBG
  printf("mmuInvBranchPredictorArray: ignored" EOL);
#endif
  // ignore (see Cortex-A8 TRM)
#else
#ifdef MMU_DBG
  printf("mmuInvBranchPredictorArray" EOL);
#endif
  asm ("mcr p15, 0, %0, c7, c5, 6": : "r"(0));
#endif
}


void mmuCleanDcacheByMVAtoPOC(u32int mva)
{
#ifdef MMU_DBG
  printf("mmuCleanDcacheByMVAtoPOC: Clearing dcache by MVA to POC %08x" EOL, mva);
#endif
  __asm__ __volatile__("mcr p15, 0, %0, c7, c10, 1": :"r"(mva));
}


void mmuCleanDcacheBySetWay(u32int setWay)
{
#ifdef MMU_DBG
  printf("mmuCleanDcacheBySetWay: Clearing dcache by set/way %08x" EOL, setWay);
#endif
  __asm__ __volatile__("mcr p15, 0, %0, c7, c10, 2": :"r"(setWay));
}


void mmuDataSyncBarrier()
{
#ifdef MMU_DBG
  printf("mmuDataSyncBarrier: synchronization barrier" EOL);
#endif
  asm ("mcr p15, 0, %0, c7, c10, 4": : "r"(0));
}


void mmuDataMemoryBarrier()
{
#ifdef MMU_DBG
  printf("mmuDataMemoryBarrier" EOL);
#endif
  asm ("mcr p15, 0, %0, c7, c10, 5": : "r"(0));
}


void mmuCleanDCacheByMVAtoPOU(u32int mva)
{
#ifdef MMU_DBG
  printf("mmuCleanDCacheByMVAtoPOU: clean dcache by MVA to PoU %08x" EOL, mva);
#endif
  __asm__ __volatile__("mcr p15, 0, %0, c7, c11, 1": :"r"(mva));
}


void mmuCleanInvDCacheByMVAtoPOC(u32int mva)
{
#ifdef MMU_DBG
  printf("mmuCleanInvDCacheByMVAtoPOC: clean and invalidate dcache by MVA to PoU %08x" EOL, mva);
#endif

  __asm__ __volatile__("mcr p15, 0, %0, c7, c14, 1": :"r"(mva));
}


void mmuCleanInvDCacheBySetWay(u32int setWay)
{
#ifdef MMU_DBG
  printf("mmuCleanInvDCacheBySetWay: clean and invalidate dcache by set/way %08x" EOL, setWay);
#endif

  __asm__ __volatile__("mcr p15, 0, %0, c7, c14, 2": :"r"(setWay));
}

/**
 * invalidate instruction TLB all
 **/
void mmuInvalidateITLB()
{
#ifdef MMU_DBG
  printf("mmuInvalidateITLB: invalidate iTLB" EOL);
#endif
  asm ("mcr p15, 0, r0, c8, c5, 0": :);
}


/**
 * invalidate instruction tlb by mva
 **/
void mmuInvalidateITLBbyMVA(u32int mva)
{
#ifdef MMU_DBG
  printf("mmuInvalidateITLBbyMVA: invalidate iTLB by MVA %08x" EOL, mva);
#endif
  asm ("mcr p15, 0, %0, c8, c5, 1": :"r"(mva));
}


void mmuInvalidateITLBbyASID(u32int asid)
{
#ifdef MMU_DBG
  printf("mmuInvalidateITLBbyASID: invalidate iTLB by ASID %08x" EOL, asid);
#endif
  asm ("mcr p15, 0, %0, c8, c5, 2": :"r"(asid));
}


void mmuInvalidateDTLB(void)
{
#ifdef MMU_DBG
  printf("mmuInvalidateDTLB: invalidate dTLB" EOL);
#endif
  asm ("mcr p15, 0, %0, c8, c6, 0": :"r"(0));
}


void mmuInvalidateDTLBbyMVA(u32int mva)
{
#ifdef MMU_DBG
  printf("mmuInvalidateDTLBbyMVA: invalidate dTLB by MVA %08x" EOL, mva);
#endif
  asm ("mcr p15, 0, %0, c8, c6, 1": :"r"(mva));
}


void mmuInvalidateDTLBbyASID(u32int asid)
{
#ifdef MMU_DBG
  printf("mmuInvalidateDTLBbyASID: invalidate dTLB by ASID %08x" EOL, asid);
#endif
  asm ("mcr p15, 0, %0, c8, c6, 2": :"r"(asid));
}


void mmuInvalidateUTLB()
{
#ifdef MMU_DBG
  printf("mmuInvalidateUTLB: invalidate uTLB" EOL);
#endif
  asm ("mcr p15, 0, %0, c8, c7, 0": :"r"(0));
}


/**
 * Clears any matching entries to <address> from the host TLB
 **/
void mmuInvalidateUTLBbyMVA(u32int mva)
{
#ifdef MMU_DBG
  printf("mmuInvalidateUTLBbyMVA: Invalidate UTLB by MVA %08x" EOL, mva);
#endif
  asm ("mcr p15, 0, %0, c8, c7, 1": :"r"(mva));
}


/**
 * clear and invalidate host data cache
 **/
void mmuClearDataCache(void)
{
#ifdef MMU_DBG
  printf("mmuClearDataCache: clearing host dCache" EOL);
#endif

  /* turn off L2 cache */
  l2_cache_disable();
  /* invalidate L2 cache also */
  v7_flush_dcache_all(BOARD_DEVICE_TYPE);

  // data sync
  __asm__ __volatile__("mcr p15, 0, %0, c7, c10, 4": :"r"(0));

  l2_cache_enable();
}




void mmuSetDomain(u8int domain, access_type access)
{
  //access is a two bit field 00 = no access, 01=client, 10=reserved, 11=manager
#ifdef MMU_DBG
  printf("mmuSetDomain: Setting domain: %x, with access bits %x" EOL, domain, (u8int)access);
#endif

  u32int value;
  __asm__ __volatile__("mrc p15, 0, %0, c3, c0, 0"
  : "=r"(value)
  :
  : "memory"
     );
#ifdef MMU_DBG
  printf("mmuSetDomain: Domain Register before update: %x" EOL, value);
#endif
  //clear the current domain
  u32int mask = ~(0b11 << (domain*2));
  value = value & mask;
  //Set the domain
  value = value | (access << ((domain)*2));
  __asm__ __volatile__("mcr p15, 0, %0, c3, c0, 0"
  :
  : "r"(value)
  : "memory"
     );

#ifdef MMU_DBG
  __asm__ __volatile__("mrc p15, 0, %0, c3, c0, 0"
  : "=r"(value)
  :
  : "memory"
              );
  printf("mmuSetDomain: Domain Register after update: %x" EOL, value);
#endif
}


void mmuSetTexRemap(bool enable)
{
#ifdef MMU_DBG
  printf("mmuSetTexRemap: %x" EOL, enable);
#endif

  u32int value;
  __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0"
  : "=r"(value)
  :
  : "memory"
     );
#ifdef MMU_DBG
  u32int treEnabled = ((value & 0x10000000) == 0x10000000) ? TRUE : FALSE; 
  printf("mmuSetTexRemap: currently SCTRL.TRE = %x" EOL, treEnabled);
#endif

  if (enable)
  {
    value |= 0x10000000;
  }
  else
  {
    value &= 0xEFFFFFFF;
  }

  __asm__ __volatile__("mcr p15, 0, %0, c1, c0, 0"
  :
  : "r"(value)
  : "memory"
     );

#ifdef MMU_DBG
  __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 0"
  : "=r"(value)
  :
  : "memory"
              );
  printf("mmuSetTexRemap: SCTRL after update %x" EOL, value);
#endif
}

void mmuSetContextID(u32int asid)
{
#ifdef MMU_DBG
//  printf("mmuSetContextID: %x" EOL, asid);
#endif
  __asm__ __volatile__("mcr p15, 0, %0, c13, c0, 1": :"r"(asid));
}



u32int getDFAR()
{
  u32int result;
  __asm__ __volatile__("mrc p15, 0, %0, c6, c0, 0"
  :"=r"(result)
     );
  return result;
}

DFSR getDFSR()
{
  DFSR result;
  __asm__ __volatile__("mrc p15, 0, %0, c5, c0, 0"
  :"=r"(result)
     );
  return result;
}

u32int getIFAR()
{
  u32int result;
  __asm__ __volatile__("mrc p15, 0, %0, c6, c0, 2"
  :"=r"(result)
     );
  return result;
}

IFSR getIFSR()
{
  IFSR result;
  __asm__ __volatile__("mrc p15, 0, %0, c5, c0, 1"
  :"=r"(result)
     );
  return result;
}


void mmuPageTableEdit(u32int entryAddr, u32int pageAddr)
{
  mmuCleanDcacheByMVAtoPOC(entryAddr);
  mmuDataSyncBarrier();
  mmuInvalidateUTLBbyMVA(pageAddr);
  mmuInvBranchPredictorArray();
  mmuDataSyncBarrier();
  mmuInstructionSync();
}


void printDataAbort()
{
  DFSR dfsr = getDFSR();
  u32int dfar = getDFAR();
  u32int faultStatus = dfsr.fs4 << 4 | dfsr.fs3_0;

  printf("Data Abort Address: %08x" EOL, dfar);
  printf("Fault type: ");
  printf("%s", dataAbtFaultString[dfsr.fs3_0]);
  printf(" (%x), domain %x, Write not Read: %x, External: %x" EOL,  faultStatus, dfsr.domain, dfsr.WnR, dfsr.ExT);
}

void printPrefetchAbort()
{
  IFSR ifsr = getIFSR();
  u32int ifar = getIFAR();
  u32int faultStatus = ifsr.fs3_0 | (ifsr.fs4 << 4);

  printf("Prefetch Abort Address: %08x" EOL, ifar);
  printf("Fault type: ");
  printf("%s", prefetchAbtFaultString[faultStatus]);
  printf(" (%x),  External: %x" EOL, faultStatus, ifsr.ExT);
}
