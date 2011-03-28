#include "common/debug.h"

#include "hardware/serial.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "memoryManager/addressing.h"
#include "memoryManager/cp15coproc.h"


extern GCONTXT * getGuestContext(void);

void initCRB(CREG * crb)
{
  u32int i = 0;

#ifdef COPROC_DEBUG
  serial_putstring("Initializing coprocessor reg bank @ address ");
  serial_putint((u32int)crb);
  serial_newline();
#endif

  // nullify all registers
  for (i = 0; i < MAX_CRB_SIZE; i++)
  {
    crb[i].value = 0;
    crb[i].valid = FALSE;
  }

  /* MIDR:
   * main ID register: CPU idenification including implementor code
   * read only. init to 411FC083
   * 41xxxxxx - implementer ID
   * xx1xxxxx - variant
   * xxxFxxxx - architecture
   * xxxxC08x - primary part number
   * xxxxxxx3 - revision */
  i = crbIndex(0, 0, 0, 0);
  crb[i].value = 0x411FC083;
  crb[i].valid = TRUE;

  /* CTR:
   * cache type register: information on the architecture of caches
   * read only. init to 80048004 */
  i = crbIndex(0, 0, 0, 1);
  crb[i].value = 0x80048004;
  crb[i].valid = TRUE;

  /* MMFR0:
   * memory model feature register 0
   * read only, init to 31100003 */
  i = crbIndex(0, 0, 1, 4);
  crb[i].value = 0x31100003;
  crb[i].valid = TRUE;

  /* MMFR1:
   * memory model feature register 1
   * read only, init to 20000000 */
  i = crbIndex(0, 0, 1, 5);
  crb[i].value = 0x20000000;
  crb[i].valid = TRUE;

  /* CCSIDR:
   * cache size id register: provide information about the architecture of caches
   * CSSELR selects default level 0 data cache information: 0xE007E01A
   * WT, WB, RA able, no WA. 256 sets, associativity 4, words per line 16 */
  i = crbIndex(0, 1, 0, 0);
  crb[i].value = 0xE007E01A;
  crb[i].valid = TRUE;

  /* CLIDR:
   * cache level ID register: beagle board 0x0A000023
   * lvl1 separate I&D caches, lvl2 unified cache */
  i = crbIndex(0, 1, 0, 1);
  crb[i].value = 0x0A000023;
  crb[i].valid = TRUE;
  
  /* CSSELR:
   * cache size select register: selects the current CCSIDR
   * initialize to lvl0, data cache (0x00000000) */
  i = crbIndex(0, 2, 0, 0);
  crb[i].value = 0x00000000;
  crb[i].valid = TRUE;

  /* SCTRL:
   * system control register, init to C5187A */  
  i = crbIndex(1, 0, 0, 0);
  crb[i].value = 0xC5187A;
  crb[i].valid = TRUE;

  /* TTBR0:
   * translation table base register 0. initialise to 0. */
  i = crbIndex(2, 0, 0, 0);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* TTBR1:
   * translation table base register 1. initialise to 0. */
  i = crbIndex(2, 0, 0, 1);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* TTBCR:
   * translation table base control register: determines which TTBR to use */
  i = crbIndex(2, 0, 0, 2);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* DACR:
   * domain access control register: initialise to dom0 ALL, dom1 ALL */
  i = crbIndex(3, 0, 0, 0);
  crb[i].value = 0x0000000f;
  crb[i].valid = TRUE;

  /* DFSR:
   * data fault status register: encodes information about the last dAbort
   * initialize to 0 */
  i = crbIndex(5, 0, 0, 0);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* IFSR:
   * instruction fault status register: encodes information about the last
   * prefetch abort. Initialize to 0 */
  i = crbIndex(5, 0, 0, 1);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* DFAR:
   * data fault address register: target address of last mem access that
   * caused a data abort. initialize to 0 */
  i = crbIndex(6, 0, 0, 0);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* IFAR:
   * instruction fault address register: target PC of the last instruction fetch
   * that caused a prefetch abort. Initialize to 0 */
  i = crbIndex(6, 0, 0, 2);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* ICIALLU:
   * invalidate all instruction caches to PoC, write-only */
  i = crbIndex(7, 0, 5, 0);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* ICIMVAU:
   * invalidate instruction caches by MVA to PoU, write-only */
  i = crbIndex(7, 0, 5, 1);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* BPIALL:
   * invalidate entire branch predictor array, write-only */
  i = crbIndex(7, 0, 5, 6);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DCCMVAC:
   * clean data cache line by MVA to PoC, write-only */
  i = crbIndex(7, 0, 10, 1);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DCCSW:
   * clean data cache line by set/way to PoC, write-only */
  i = crbIndex(7, 0, 10, 2);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DCCMVAU: 
   * clean data cache line by MVA to PoU, write only */
  i = crbIndex(7, 0, 11, 1);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DCCIMVAC: clean and invalidate dCache by MVA to PoC, write-only */
  i = crbIndex(7, 0, 14, 1);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DCCISW:
   * clean and invalidate data cache line by set/way, write-only */
  i = crbIndex(7, 0, 14, 2);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* ITLBIALL: invalide instruction TLB (all), write-only */
  i = crbIndex(8, 0, 5, 0);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* ITLBIMVA: invalide instruction TLB by MVA, write-only */
  i = crbIndex(8, 0, 5, 1);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* ITLBIASID: invalide instruction TLB by ASID match, write-only */
  i = crbIndex(8, 0, 5, 2);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DTLBIALL: invalide data TLB (all), write-only */
  i = crbIndex(8, 0, 6, 0);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DTLBIMVA: invalide data TLB by MVA, write-only */
  i = crbIndex(8, 0, 6, 1);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* DTLBIMVA: invalide data TLB by ASID match, write-only */
  i = crbIndex(8, 0, 6, 2);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* TLBIALL: invalide unified TLB, write-only */
  i = crbIndex(8, 0, 7, 0);
  crb[i].value = 0;
  crb[i].valid = TRUE;

  /* PRRR:
   * primary region remap register: does lots of weird things with
   * C, B, and TEX attributes of memory regions, init to 0x98AA4 */
  i = crbIndex(10, 0, 2, 0);
  crb[i].value = 0x98AA4;
  crb[i].valid = TRUE;

  /* NMRR:
   * normal memory remap register: additional mapping controls for memory regions
   * that are mapped as Normal memory by their entry in the PRRR
   * initialize to 44E048E0 */
  i = crbIndex(10, 0, 2, 1);
  crb[i].value = 0x44E048E0;
  crb[i].valid = TRUE;
  
  /* FCSEIDR: 
   * fast context switch extension process ID register
   * initialize to 0 */
  i = crbIndex(13, 0, 0, 0);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* CONTEXTID:
   * context ID register 
   * initialize to 0 */
  i = crbIndex(13, 0, 0, 1);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* TPIDRURW:
   * software thread ID register, user mode read-write 
   * initialize to 0 */
  i = crbIndex(13, 0, 0, 2);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* TPIDRURO:
   * software thread ID register, user mode read-only
   * initialize to 0 */
  i = crbIndex(13, 0, 0, 3);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;

  /* TPIDRPRW: 
   * software thread ID register, privileged read/write only
   * initialize to 0 */
  i = crbIndex(13, 0, 0, 4);
  crb[i].value = 0x0;
  crb[i].valid = TRUE;
}

void setCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr, u32int val)
{
  u32int index = crbIndex(CRn, opc1, CRm, opc2);
  
  if (!crbPtr[index].valid)
  {
    // guest writing to a register that is not valid yet! investigate
    serial_putstring("setCregVal (CRn=");
    serial_putint(CRn);
    serial_putstring(" opc1=");
    serial_putint(opc1);
    serial_putstring(" CRm=");
    serial_putint(CRm);
    serial_putstring(" opc2=");
    serial_putint(opc2);
    serial_putstring(") Value = ");
    serial_putint(val);
    serial_newline();
    DIE_NOW(0, "setCregVal: writing to uninitialized register. investigate.");
  }
  u32int oldVal = crbPtr[index].value;
  crbPtr[index].value = val;
  // if we are writting to this register, it's probably valid already!
  crbPtr[index].valid = TRUE;

#ifdef COPROC_DEBUG
  serial_putstring("setCregVal (CRn=");
  serial_putint(CRn);
  serial_putstring(" opc1=");
  serial_putint(opc1);
  serial_putstring(" CRm=");
  serial_putint(CRm);
  serial_putstring(" opc2=");
  serial_putint(opc2);
  serial_putstring(") Value = ");
  serial_putint(val);
  serial_newline();
#endif

  /* probably a better place to put these checks */
  if (CRn==0 && opc1==1 && CRm==0 && opc2==0)
  {
    DIE_NOW(0, "setCregVal: writing to CCSIDR - read-only");
  }
  else if (CRn==0 && opc1==1 && CRm==0 && opc2==1)
  {
    DIE_NOW(0, "setCregVal: writing to CLIDR - read-only");
  }
  else if (CRn==0 && opc1==2 && CRm==0 && opc2==0)
  {
    // write to CSSELR: cache size select register.
    // need to update R/O CSSIDR value
    u32int newCCSIDR = 0;
    switch (val)
    {
      case 0:
      {
        // 0b0000 - lvl1 data cache
        newCCSIDR = 0xE007E01A;
        break;
      }
      case 1:
      {
        // 0b0001 - lvl1 instruction cache
        newCCSIDR = 0x2007E01A;
        break;
      } 
      case 2:
      {
        // 0b0010 - lvl2 unified cache
        newCCSIDR = 0xF03FE03A;
        break;
      }
      default:
      {
        // no such cache exists on the beagleboard!
        serial_putstring("setCregVal: CSSELR = ");
        serial_putint(val);
        serial_newline();
        DIE_NOW(0, "setCregVal: CSSELR selects an unimplemented cache."); 
      }
    } // switch (value) ends
    // update CCSIDR with correct value
    crbPtr[crbIndex(0, 1, 0, 0)].value = newCCSIDR;
  }
  else if (CRn==1 && opc1==0 && CRm==0 && opc2==0)
  {
#ifdef COPROC_DEBUG
    serial_putstring("setCregVal: sys ctrl reg: ");
    serial_putint(val);
    serial_newline();
#endif

    // SCTRL: System control register
    // check for MMU enable
    if( (0 == (oldVal & 0x1)) && (1 == (val & 0x1)) )
    {
#ifdef COPROC_DEBUG
      serial_putstring("MMU enable.");
      serial_newline();
#endif
      guestEnableVirtMem();
    }
#ifdef COPROC_DEBUG
    else if((1 == (oldVal & 0x1)) && (0 == (val & 0x1)))
    {
      serial_putstring("MMU disable.");
      serial_newline();
    }
#endif
    //Interupt handler remap
    if( (0 == (oldVal & 0x2000)) && (0 != (val & 0x2000)) )
    {
#ifdef COPROC_DEBUG
      serial_putstring("CP15: high interrupt vector set.");
      serial_newline();
#endif
      (getGuestContext())->guestHighVectorSet = TRUE; 
    }
  }
  else if (CRn==2 && opc1==0 && CRm==0 && opc2==0)
  {
#ifdef COPROC_DEBUG
    serial_putstring("setCregVal: TTBR0 write ");
    serial_putint(val);
    serial_newline();
#endif
    initialiseGuestShadowPageTable(val);
  }
  else if (CRn==2 && opc1==0 && CRm==0 && opc2==1)
  {
    serial_putstring("setCregVal: WARN: TTBR1 write ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==2 && opc1==0 && CRm==0 && opc2==2)
  {
    // TTBCR: translation table base control register
    // 0b000 - always use TTBR0. !0 - depends...
    if ((val & 0x7) != 0)
    {
      DIE_NOW(0, "setCregVal: TTBCR needs to select translation table base!");
    }
  }
  else if (CRn == 3 && opc1 == 0 && CRm==0 && opc2==0)
  {
    if (oldVal != val)
    {
#ifdef COPROC_DEBUG
      serial_putstring("CP15: Domain Access Control Register write val ");
      serial_putint(val);
      serial_putstring(" old DACR ");
      serial_putint(oldVal);
      serial_newline();
#endif
      changeGuestDomainAccessControl(oldVal, val);
    }
  }
#ifdef COPROC_DEBUG
  else if (CRn==5 && opc1==0 && CRm==0 && opc2==0)
  {
    // DFSR: data fault status register
    serial_putstring("setCregVal: set DFSR to ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==5 && opc1==0 && CRm==0 && opc2==1)
  {
    // IFSR: instruction fault status register
    serial_putstring("setCregVal: set IFSR to ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==6 && opc1==0 && CRm==0 && opc2==0)
  {
    // DFAR: data fault address register
    serial_putstring("setCregVal: set DFAR to ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==6 && opc1==0 && CRm==0 && opc2==2)
  {
    // IFAR: instruction fault address register
    serial_putstring("setCregVal: set IFAR to ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==5 && opc2==0)
  {
    // ICIALLU: invalidate all instruction caches to PoC
    serial_putstring("setCregVal: invalidate all iCaches to PoC");
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==5 && opc2==1)
  {
    // ICIMVAU: invalidate instruction caches by MVA to PoU
    serial_putstring("setCregVal: invalidate iCaches by MVA to PoC: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==5 && opc2==6)
  {
    // BPIALL: invalidate entire branch predictor array
    serial_putstring("setCregVal: invalidate entire branch predictor array");
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==11 && opc2==1)
  {
    // DCCMVAU: clean data cache line by MVA to PoU
    serial_putstring("setCregVal: clean dCache line by MVA to PoU: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==14 && opc2==1)
  {
    // DCCIMVAC: clean and invalidate dCache by MVA to PoC
    serial_putstring("setCregVal: clean and invalidate dCache by MVA to PoC: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==14 && opc2==2)
  {
    // DCCISW: clean and invalidate data cache line by set/way
    serial_putstring("setCregVal: clean and invalidate Dcache line, set/way: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==10 && opc2==1)
  {
    // DCCMVAC: clean data cache line by MVA to PoC
    serial_putstring("setCregVal: clean Dcache line by MVA to PoC: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==7 && opc1==0 && CRm==10 && opc2==2)
  {
    // DCCSW: clean data cache line by set/way to PoC 
    serial_putstring("setCregVal: clean Dcache line, set/way to PoC: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==5 && opc2==0)
  {
    // ITLBIALL: invalide instruction TLB (all)
    serial_putstring("setCregVal: invalidate instruction TLB (all)");
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==5 && opc2==1)
  {
    // ITLBIALL: invalide instruction TLB by MVA
    serial_putstring("setCregVal: invalidate instruction TLB by MVA: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==5 && opc2==2)
  {
    // ITLBIASID: invalide instruction TLB by ASID match 
    serial_putstring("setCregVal: invalidate instruction TLB by ASID match: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==6 && opc2==0)
  {
    // DTLBIALL: invalide data TLB (all)
    serial_putstring("setCregVal: invalidate data TLB (all)");
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==6 && opc2==1)
  {
    // DTLBIMVA: invalidate dTLB entry by MVA
    serial_putstring("setCregVal: invalidate data TLB by MVA: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==6 && opc2==2)
  {
    // DTLBIASID: invalidate dTLB entry by MVA
    serial_putstring("setCregVal: invalidate data TLB by ASID match: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==7 && opc2==0)
  {
    // TLBIALL: invalide unified TLB, write-only
    serial_putstring("setCregVal: invalidate unified TLB (all)");
    serial_newline();
  }
#endif
  else if (CRn==10 && opc1==0 && CRm==2 && opc2==0)
  {
    // PRRR: primary region remap register
    serial_putstring("setCregVal: WARN: PRRR value ");
    serial_putint(val);
    serial_newline();
    asm("mcr p15, 0, %0, c10, c2, 0"
    :
    :"r"(val)
    :"memory"
       );
  }
  else if (CRn==10 && opc1==0 && CRm==2 && opc2==1)
  {
    // NMRR: normal memory remap register
    serial_putstring("setCregVal: WARN: NMRR value ");
    serial_putint(val);
    serial_newline();
    asm("mcr p15, 0, %0, c10, c2, 1"
    :
    :"r"(val)
    :"memory"
       );
  }
  else if (CRn==13 && opc1==0 && CRm==0 && opc2==0)
  {
    // FCSEIDR: fast context switch extension process ID register
    DIE_NOW(0, "setCregVal: writing FCSEIDR - investigate!");
  }
  else if (CRn==13 && opc1==0 && CRm==0 && opc2==1)
  {
    // CONTEXTID: context ID register 
    if (val != 0)
    {
      serial_putstring("setCregVal: WARN: CONTEXTID value ");
      serial_putint(val);
      serial_newline();
    }
    asm("mcr p15, 0, %0, c13, c0, 1"
    :
    :"r"(val)
    :"memory"
       );
  }
  else if (CRn==13 && opc1==0 && CRm==0 && opc2==2)
  {
    // TPIDRURW: software thread ID register, user mode read-write 
    if (val != 0)
    {
      serial_putstring("setCregVal: WARN: TPIDRURW value ");
      serial_putint(val);
      serial_newline();
    }
  }
  else if (CRn==13 && opc1==0 && CRm==0 && opc2==3)
  {
    // TPIDRURO: software thread ID register, user mode read-only
    // writes are caught by the hypervisor - they are privileged operations
    // but reads are not! must propagate TPIDRURO value to real CP15.
#ifdef COPROC_DEBUG
    serial_putstring("setCregVal: WARN: TPIDRURO value ");
    serial_putint(val);
    serial_newline();
#endif
    asm("mcr p15, 0, %0, c13, c0, 3"
    :
    :"r"(val)
    :"memory"
       );
  }
  else if (CRn==13 && opc1==0 && CRm==0 && opc2==4)
  {
    // TPIDRPRW: software thread ID register, user mode no access 
    if (val != 0)
    {
      serial_putstring("setCregVal: WARN: TPIDRPRW value ");
      serial_putint(val);
      serial_newline();
    }
  }
}


u32int getCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr)
{
  CREG reg;
  u32int index = crbIndex(CRn, opc1, CRm, opc2);
  reg = crbPtr[index];

#ifdef COPROC_DEBUG
  serial_putstring("getCreg (CRn=");
  serial_putint(CRn);
  serial_putstring(" opc1=");
  serial_putint(opc1);
  serial_putstring(" CRm=");
  serial_putint(CRm);
  serial_putstring(" opc2=");
  serial_putint(opc2);
  serial_putstring(") Value = ");
  serial_putint(reg.value);
  serial_newline();
#endif

  if (reg.valid)
  {
    return reg.value;
  }
  else
  {
    serial_putstring("getCreg (CRn=");
    serial_putint(CRn);
    serial_putstring(" opc1=");
    serial_putint(opc1);
    serial_putstring(" CRm=");
    serial_putint(CRm);
    serial_putstring(" opc2=");
    serial_putint(opc2);
    serial_putstring(") Value = ");
    serial_putint(reg.value);
    serial_newline();
    DIE_NOW(0, "Undefined CP15 register!");
    return 0;
  }
}

u32int crbIndex(u32int CRn, u32int opc1, u32int CRm, u32int opc2)
{
  u32int index = 0;
  // value 0 to 7
  u32int indexOpc2 = opc2;
  // values 0, 8, 16... to 120 ( 16 increments of 8)
  u32int indexCRm  = CRm  * MAX_OPC2_VALUES;
  // values 0, 128, 256, 384... 894 (8 increments of 128)
  u32int indexOpc1 = opc1 * MAX_CRM_VALUES * MAX_OPC2_VALUES;
  // values 0, 1024, 2048, 3072, 4096... 15360 (16 increments of 1024)
  u32int indexCRn  = CRn * MAX_OPC1_VALUES * MAX_CRM_VALUES * MAX_OPC2_VALUES;

  index = indexCRn + indexOpc1 + indexCRm + indexOpc2;
  return index;
}
