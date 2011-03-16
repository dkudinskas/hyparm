#include "cp15coproc.h"
#include "commonInstrFunctions.h"
#include "debug.h"
#include "addressing.h"

extern GCONTXT * getGuestContext(void);

void initCRB(CREG * crb)
{
  u32int i = 0;
  u32int physVal = 0;
  u16int CRn = 0; u8int opc1 = 0; u16int CRm = 0; u8int opc2 = 0;

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

  /* initialize default register values */
  CRn = 0; opc1 = 0; CRm = 0; opc2 = 0;
  asm ("mrc p15, 0, %0, c0, c0, 0"
       : "=r"(physVal)            /* output operands */
       :                          /* input operands */
       : "memory"                 /* clobbered registers */
       );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 0; opc1 = 0; CRm = 0; opc2 = 1;
  asm ("mrc p15, 0, %0, c0, c0, 1"
       : "=r"(physVal)            /* output operands */
       :                          /* input operands */
       : "memory"                 /* clobbered registers */
       );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 0; opc1 = 0; CRm = 1; opc2 = 4;
  asm ("mrc p15, 0x00, %0, c0, c1, 0x04"
       : "=r"(physVal)            /* output operands */
       :                          /* input operands */
       : "memory"                 /* clobbered registers */
       );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 0; opc1 = 0; CRm = 1; opc2 = 5;
  asm ("mrc p15, 0x00, %0, c0, c1, 0x05"
       : "=r"(physVal)            /* output operands */
       :                          /* input operands */
       : "memory"                 /* clobbered registers */
       );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 0; opc1 = 1; CRm = 0; opc2 = 0;
  asm ("mrc p15, 0x01, %0, c0, c0, 0x00"
       : "=r"(physVal)            /* output operands */
       :                          /* input operands */
       : "memory"                 /* clobbered registers */
       );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 0; opc1 = 1; CRm = 0; opc2 = 1;
  asm ("mrc p15, 0x01, %0, c0, c0, 0x01"
       : "=r"(physVal)            /* output operands */
       :                          /* input operands */
       : "memory"                 /* clobbered registers */
       );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 1; opc1 = 0; CRm = 0; opc2 = 0;
  asm ("mrc p15, 0x00, %0, c1, c0, 0x00"
       : "=r"(physVal)            /* output operands */
       :                          /* input operands */
       : "memory"                 /* clobbered registers */
       );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 0; opc1 = 0; CRm = 1; opc2 = 5;
  asm ("mrc p15, 0x00, %0, c0, c1, 0x05"
  : "=r"(physVal)            /* output operands */
  :                          /* input operands */
  : "memory"                 /* clobbered registers */
      );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

//  CRn = 0; opc1 = 1; CRm = 0; opc2 = 1;
//  asm ("mrc p15, 0x01, %0, c0, c0, 0x01"
//  : "=r"(physVal)            /* output operands */
//  :                          /* input operands */
//  : "memory"                 /* clobbered registers */
//      );
//  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
//  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

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

  /* CCSIDR:
   * cache size id register: provide information about the architecture of caches
   * CSSELR selects default level 0 data cache information: 0xE007E01A
   * WT, WB, RA able, no WA. 256 sets, associativity 4, words per line 16 */
  i = crbIndex(0, 1, 0, 0);
  crb[i].value = 0xE007E01A;
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

  /* DCCISW:
   * clean and invalidate data cache line by set/way, W/O register */
  i = crbIndex(7, 0, 14, 2);
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
//    DIE_NOW(0, "setCregVal: writing to uninitialized register. investigate.");
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
  else if (CRn==2 && opc1==0 && CRm==0 && opc2==0)
  {
//#ifdef COPROC_DEBUG
    serial_putstring("setCregVal: TTBR0 write ");
    serial_putint(val);
    serial_newline();
//#endif
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
  else if (CRn==7 && opc1==0 && CRm==14 && opc2==2)
  {
    // DCCISW: clean and invalidate data cache line by set/way
    serial_putstring("setCregVal: clean and invalidate Dcache line, set/way: ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==8 && opc1==0 && CRm==7 && opc2==0)
  {
    // TLBIALL: invalide unified TLB, write-only
    serial_putstring("setCregVal: invalidate unified TLB (all)");
    serial_newline();
  }
  else if (CRn==10 && opc1==0 && CRm==2 && opc2==0)
  {
    // PRRR: primary region remap register
    serial_putstring("setCregVal: WARN: PRRR value ");
    serial_putint(val);
    serial_newline();
  }
  else if (CRn==10 && opc1==0 && CRm==2 && opc2==1)
  {
    // NMRR: normal memory remap register
    serial_putstring("setCregVal: WARN: NMRR value ");
    serial_putint(val);
    serial_newline();
  }
#endif
  else if(CRn==1 && CRm==0 && opc2==0)
  {
    // Test Sys ctrl for mmu enable
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
