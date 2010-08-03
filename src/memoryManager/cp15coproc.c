#include "cp15coproc.h"
#include "commonInstrFunctions.h"
#include "addressing.h"

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

  CRn = 0; opc1 = 1; CRm = 0; opc2 = 1;
  asm ("mrc p15, 0x01, %0, c0, c0, 0x01"
  : "=r"(physVal)            /* output operands */
  :                          /* input operands */
  : "memory"                 /* clobbered registers */
      );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;

  CRn = 0; opc1 = 1; CRm = 0; opc2 = 0;
  asm ("mrc p15, 0x00, %0, c0, c0, 0x01"
  : "=r"(physVal)            /* output operands */
  :                          /* input operands */
  : "memory"                 /* clobbered registers */
      );
  crb[crbIndex(CRn, opc1, CRm, opc2)].value = physVal;
  crb[crbIndex(CRn, opc1, CRm, opc2)].valid = TRUE;
}

void setCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr, u32int val)
{
  int index = crbIndex(CRn, opc1, CRm, opc2);
  u32int oldVal = crbPtr[index].value;
  crbPtr[index].value = val;

#ifdef COPROC_DEBUG
  serial_putstring("setCreg (CRn=");
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

  //Check TTBR0
  if(CRn==2 && opc2==0)
  {
#ifdef COPROC_DEBUG
    serial_putstring("TTBR0 write");
    serial_newline();
#endif
    initialiseGuestShadowPageTable(val);
  }
  //Test Sys ctrl for mmu enable
  else if(CRn==1 && CRm==0 && opc2==0)
  {
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
      //Want to know about this!
      serial_putstring("Interupt handler remap bit set (cp15coproc.c)");
      serial_newline();
    }
  }
}


u32int getCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr)
{
  CREG reg;
  int index = crbIndex(CRn, opc1, CRm, opc2);
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
    error_function("Undefined CP15 register!", 0);
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
