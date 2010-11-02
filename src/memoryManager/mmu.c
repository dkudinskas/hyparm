#include "mmu.h"
#include "serial.h"

//uncomment to enable debug #define MMU_DEBUG

static const char* fault_string[] =
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

void clearTLB(void);
void clearCache(void);
void setTTBCR(u32int value);

void mmu_compile_time_check(void)
{
  COMPILE_TIME_ASSERT((sizeof(IFSR) == sizeof(u32int)) , _IFSR_struct_not_32bit);
  COMPILE_TIME_ASSERT((sizeof(DFSR) == sizeof(u32int)) , _DFSR_struct_not_32bit);
}

void mmuInit()
{
#ifdef MMU_DEBUG
  serial_putstring("MMU init");
  serial_newline();
#endif

  dataBarrier();
  clearTLB();
  clearCache();
  setTTBCR(0);
}

void dataBarrier()
{
#ifdef MMU_DEBUG
  serial_putstring("Data Barrier");
  serial_newline();
#endif
    //It doesn't matter which register it written/nor the value inside it
  asm ("mcr p15, 0, r0, c7, c10, 5"
  :
  :
  :"memory"
      );
}

void clearTLB()
{
#ifdef MMU_DEBUG
  serial_putstring("Clearing TLB");
  serial_newline();
#endif
  // mcr coproc opc1 Rt CRn CRm opc2
  //It doesn't matter which register it written/nor the value inside it
  asm ("mcr p15, 0, r0, c8, c7, 0"
  :
  :
  :"memory"
      );
}

//Clears any matching entries to <address> from the TLB
void clearTLBbyMVA(u32int address)
{
  address &= 0xFFFFF000;
#ifdef MMU_DEBUG
  serial_putstring("Clearing TLB of virt addr: 0x");
  serial_putint(address);
  serial_newline();
#endif
  // mcr coproc opc1 Rt CRn CRm opc2
  asm ("mcr p15, 0, %0, c8, c7, 1"
  :
  :"r"(address)
  :"memory"
      );
}

void clearCache()
{
#ifdef MMU_DEBUG
  serial_putstring("Clearing caches");
  serial_newline();
#endif
  asm ("mcr p15, 0, r0, c7, c5, 0"
  :
  :
  :"memory"
      );
}

//Need to set range of TTBCR

void setTTBCR(u32int value)
{
  asm("mcr p15, 0, %0, c2, c0, 0"
  :
  :"r"(value)
  :"memory"
     );
}

void setDomain(u8int domain, access_type access)
{
  //access is a two bit field 00 = no access, 01=client, 10=reserved, 11=manager
#ifdef MMU_DEBUG
  serial_putstring("Setting domain: ");
  serial_putint_nozeros(domain);
  serial_putstring(", with access bits 0x");
  serial_putint_nozeros((u8int)access);
  serial_newline();
#endif

  u32int value;
  asm volatile("mrc p15, 0, %0, c3, c0, 0"
  : "=r"(value)
  :
  : "memory"
     );
#ifdef MMU_DEBUG
  serial_putstring("Domain Register before update: 0x");
  serial_putint(value);
  serial_newline();
#endif
  //clear the current domain
  u32int mask = ~(0b11 << (domain*2));
  value = value & mask;
  //Set the domain
  value = value | (access << ((domain)*2));
  asm volatile("mcr p15, 0, %0, c3, c0, 0"
  :
  : "r"(value)
  : "memory"
     );

#ifdef MMU_DEBUG
  asm volatile("mrc p15, 0, %0, c3, c0, 0"
  : "=r"(value)
  :
  : "memory"
              );
  serial_putstring("Domain Register after update: 0x");
  serial_putint(value);
  serial_newline();
#endif

}

void mmuInsertPt0(descriptor* addr)
{
#ifdef MMU_DEBUG
  serial_putstring("Add entry into TTBR0: 0x");
  serial_putint((u32int)addr);
#endif
  //TODO: need to improve this to insert the correct bit masks
  asm("mcr p15, 0, %0,c2,c0,0"
  :
  :"r"(addr)
     );
}

void mmuInsertPt1(descriptor* addr)
{
#ifdef MMU_DEBUG
  serial_putstring("Add entry into TTBR1: 0x");
  serial_putint((u32int)addr);
#endif
  //TODO: need to improve this to insert the correct bit masks
  asm("mcr p15, 0, %0,c2,c0,1"
  :
  :"r"(addr)
     );
}

descriptor* mmuGetPt0()
{
  u32int regVal = 0;
  //TODO: need to improve this to insert the correct bit masks
  asm("mrc p15, 0, %0,c2,c0,0"
      :"=r"(regVal)
      :
      );
#ifdef MMU_DEBUG
  serial_putstring("Get TTBR0: 0x");
  serial_putint((u32int)regVal);
#endif
  return (descriptor*)regVal;
}



void mmuEnableVirtAddr()
{
  u32int tempReg;
  //This may need a bit of investigation/logic to ensure the bit masks we set are correct
  asm volatile("mrc p15, 0, %0, c1, c0, 0\n\t"
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
  asm volatile("mrc p15, 0, %0, c1, c0, 0\n\t"
               "BIC %0, %0, #5\n\t" //enable MMU & Caching
               "mcr p15, 0, %0, c1, c0, 0\n\t"
//               "mcr p15, 0, %0, c7, c5, 4\n\t" //ISB
  :"=r"(tempReg)
  :
  : "memory"
     );
}



u32int getDFAR()
{
  u32int result;
  asm("mrc p15, 0, %0, c6, c0, 0"
  :"=r"(result)
     );
  return result;
}

DFSR getDFSR()
{
  DFSR result;
  asm("mrc p15, 0, %0, c5, c0, 0"
  :"=r"(result)
     );
  return result;
}

u32int getIFAR()
{
  u32int result;
  asm("mrc p15, 0, %0, c6, c0, 2"
  :"=r"(result)
     );
  return result;
}

IFSR getIFSR()
{
  IFSR result;
  asm("mrc p15, 0, %0, c5, c0, 1"
  :"=r"(result)
     );
  return result;
}

void printDataAbort()
{
  DFSR dfsr = getDFSR();
  u32int dfar = getDFAR();

  serial_putstring("Data Abort Address: 0x");
  serial_putint(dfar);
  serial_newline();

  serial_putstring("Fault type: ");
  serial_putstring((char*)fault_string[dfsr.fs3_0]);
  serial_putstring(" (0x");
  u32int faultStatus = dfsr.fs4 << 4 | dfsr.fs3_0;
  serial_putint_nozeros(faultStatus);
  serial_putstring(")");
  serial_newline();

  serial_putstring("domain: 0x");
  serial_putint_nozeros(dfsr.domain);

  /* Perhaps read out the domain and spit out the permission bits set for that domain at this point? */

  serial_putstring(", Write not Read: 0x");
  serial_putint_nozeros(dfsr.WnR);

  serial_putstring(", External: 0x");
  serial_putint_nozeros(dfsr.ExT);
  serial_newline();

}

void printPrefetchAbort()
{
  IFSR ifsr = getIFSR();
  u32int ifar = getIFAR();

  serial_putstring("Prefetch Abort Address: 0x");
  serial_putint(ifar);
  serial_newline();

  serial_putstring("Fault type: ");
  serial_putstring((char*)fault_string[ifsr.fs3_0]);
  serial_putstring(" (0x");
  u32int faultStatus = ifsr.fs4 << 4 | ifsr.fs3_0;
  serial_putint_nozeros(faultStatus);
  serial_putstring(")");

  serial_putstring("External: 0x");
  serial_putint_nozeros(ifsr.ExT);
  serial_newline();
}
