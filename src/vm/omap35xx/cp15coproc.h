#ifndef __VM__OMAP_35XX__CP15_COPROC_H__
#define __VM__OMAP_35XX__CP15_COPROC_H__

#include "common/compiler.h"
#include "common/types.h"


// uncomment me for COPROCESSOR DEBUG: #define COPROC_DEBUG


#define MAX_CRN_VALUES    16
#define MAX_OPC1_VALUES   8
#define MAX_CRM_VALUES    16
#define MAX_OPC2_VALUES   8
#define MAX_CRB_SIZE      MAX_CRN_VALUES * MAX_OPC1_VALUES * MAX_CRM_VALUES * MAX_OPC2_VALUES

#define SYS_CTRL_MMU_ENABLE          0x00000001
#define SYS_CTRL_ALIGNMENT           0x00000002
#define SYS_CTRL_CACHING             0x00000004
#define SYS_CTRL_SWAP_INSTR          0x00000400
#define SYS_CTRL_BRANCH_PRED         0x00000800
#define SYS_CTRL_ICACHING            0x00001000
#define SYS_CTRL_HIGH_VECS           0x00002000
#define SYS_CTRL_RR_CACHE            0x00004000
#define SYS_CTRL_HW_ACC_FLAG         0x00020000
#define SYS_CTRL_FIQ                 0x00200000
#define SYS_CTRL_VECT_INTERRUPT      0x01000000
#define SYS_CTRL_EXC_BIG_END         0x02000000
#define SYS_CTRL_NON_MASK_FIQ        0x08000000
#define SYS_CTRL_TEX_REMAP           0x10000000
#define SYS_CTRL_ACCESS_FLAG         0x20000000
#define SYS_CTRL_THUMB_EXC_HANDLE    0x40000000


#define CRB_INDEX(CRn, opc1, CRm, opc2)                                                            \
  ((((CRn * MAX_OPC1_VALUES) + opc1) * MAX_CRM_VALUES + CRm) * MAX_OPC2_VALUES + opc2)


typedef enum coprocessor15Register
{
  CP15_MIDR      = CRB_INDEX( 0, 0,  0, 0),
  CP15_CTR       = CRB_INDEX( 0, 0,  0, 1),
  CP15_MMFR0     = CRB_INDEX( 0, 0,  1, 4),
  CP15_MMFR1     = CRB_INDEX( 0, 0,  1, 5),
  CP15_CCSIDR    = CRB_INDEX( 0, 1,  0, 0),
  CP15_CLIDR     = CRB_INDEX( 0, 1,  0, 1),
  CP15_CSSELR    = CRB_INDEX( 0, 2,  0, 0),
  CP15_SCTRL     = CRB_INDEX( 1, 0,  0, 0),
  CP15_TTBR0     = CRB_INDEX( 2, 0,  0, 0),
  CP15_TTBR1     = CRB_INDEX( 2, 0,  0, 1),
  CP15_TTBCR     = CRB_INDEX( 2, 0,  0, 2),
  CP15_DACR      = CRB_INDEX( 3, 0,  0, 0),
  CP15_DFSR      = CRB_INDEX( 5, 0,  0, 0),
  CP15_IFSR      = CRB_INDEX( 5, 0,  0, 1),
  CP15_DFAR      = CRB_INDEX( 6, 0,  0, 0),
  CP15_IFAR      = CRB_INDEX( 6, 0,  0, 2),
  CP15_ICIALLU   = CRB_INDEX( 7, 0,  5, 0),
  CP15_ICIMVAU   = CRB_INDEX( 7, 0,  5, 1),
  CP15_ISB       = CRB_INDEX( 7, 0,  5, 4),
  CP15_BPIALL    = CRB_INDEX( 7, 0,  5, 6),
  CP15_DCCMVAC   = CRB_INDEX( 7, 0, 10, 1),
  CP15_DCCSW     = CRB_INDEX( 7, 0, 10, 2),
  CP15_DSB       = CRB_INDEX( 7, 0, 10, 4),
  CP15_DMB       = CRB_INDEX( 7, 0, 10, 5),
  CP15_DCCMVAU   = CRB_INDEX( 7, 0, 11, 1),
  CP15_DCCIMVAC  = CRB_INDEX( 7, 0, 14, 1),
  CP15_DCCISW    = CRB_INDEX( 7, 0, 14, 2),
  CP15_ITLBIALL  = CRB_INDEX( 8, 0,  5, 0),
  CP15_ITLBIMVA  = CRB_INDEX( 8, 0,  5, 1),
  CP15_ITLBIASID = CRB_INDEX( 8, 0,  5, 2),
  CP15_DTLBIALL  = CRB_INDEX( 8, 0,  6, 0),
  CP15_DTLBIMVA  = CRB_INDEX( 8, 0,  6, 1),
  CP15_DTLBIASID = CRB_INDEX( 8, 0,  6, 2),
  CP15_TLBIALL   = CRB_INDEX( 8, 0,  7, 0),
  CP15_TLBIMVA   = CRB_INDEX( 8, 0,  7, 1),
  CP15_PRRR      = CRB_INDEX(10, 0,  2, 0),
  CP15_NMRR      = CRB_INDEX(10, 0,  2, 1),
  CP15_VBAR      = CRB_INDEX(12, 0,  0, 0),
  CP15_FCSEIDR   = CRB_INDEX(13, 0,  0, 0),
  CP15_CONTEXTID = CRB_INDEX(13, 0,  0, 1),
  CP15_TPIDRURW  = CRB_INDEX(13, 0,  0, 2),
  CP15_TPIDRURO  = CRB_INDEX(13, 0,  0, 3),
  CP15_TPIDRPRW  = CRB_INDEX(13, 0,  0, 4)
} Coprocessor15Register;

typedef struct coprocessorRegisterBankEntry
{
  u32int value;
  bool   valid;
} CREG;


CREG *createCRB(void) __cold__;
u32int getCregVal(CREG *registerBank, u32int registerIndex);
void setCregVal(CREG *registerBank, u32int registerIndex, u32int value);


#endif /* __VM__OMAP_35XX__CP15_COPROC_H__ */
