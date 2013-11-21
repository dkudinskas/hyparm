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

#define IDPFR0_ARM_ISA_SUPPORT       0x00000001
#define IDPFR0_THUMB2_ISA_SUPPORT    0x00000030


#define CRB_INDEX(CRn, opc1, CRm, opc2)                                                            \
  ((((CRn * MAX_OPC1_VALUES) + opc1) * MAX_CRM_VALUES + CRm) * MAX_OPC2_VALUES + opc2)

#define CRB_INDEX_TO_CRM(index)  ((index >> 3) & 0xF)
#define CRB_INDEX_TO_CRN(index)  ((index >> 10) & 0xF)
#define CRB_INDEX_TO_OP1(index)  ((index >> 7) & 0x7)
#define CRB_INDEX_TO_OP2(index)  (index & 0x7)


typedef enum coprocessor15Register
{
  CP15_MIDR      = CRB_INDEX( 0, 0,  0, 0),
  CP15_CTR       = CRB_INDEX( 0, 0,  0, 1),
  CP15_IDPFR0    = CRB_INDEX( 0, 0,  1, 0),
  CP15_MMFR0     = CRB_INDEX( 0, 0,  1, 4),
  CP15_MMFR1     = CRB_INDEX( 0, 0,  1, 5),
  CP15_CCSIDR    = CRB_INDEX( 0, 1,  0, 0),
  CP15_CLIDR     = CRB_INDEX( 0, 1,  0, 1),
  CP15_CSSELR    = CRB_INDEX( 0, 2,  0, 0),
  CP15_SCTRL     = CRB_INDEX( 1, 0,  0, 0),
  CP15_ACTLR     = CRB_INDEX( 1, 0,  0, 1),
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
  CP15_DCIMVAC   = CRB_INDEX( 7, 0,  6, 1),
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

/* Cortex-A8 specific */
struct auxiliaryControlRegister
{
  unsigned l1AliasChecksEnable : 1;
  unsigned l2Enable : 1;
  unsigned : 1;
  unsigned l1ParityDetectEnable : 1;
  unsigned axiSpeculativeAccessEnable : 1;
  unsigned l1NEONEnable : 1;
  unsigned invalidateBTBEnable : 1;
  unsigned branchSizeMispredictDisable : 1;
  unsigned wfiNOP : 1;
  unsigned pldNOP : 1;
  unsigned forceSingleIssue : 1;
  unsigned forceLoadStoreSingleIssue : 1;
  unsigned forceNEONSingleIssue : 1;
  unsigned forceMainClock : 1;
  unsigned forceNEONClock : 1;
  unsigned forceETMClock : 1;
  unsigned cp1415PipelineFlush : 1;
  unsigned cp1415WaitOnIdle : 1;
  unsigned cp1415InstructionSerialization : 1;
  unsigned clockStopRequestDisable : 1;
  unsigned cacheMaintenancePipeline : 1;
  unsigned : 9;
  unsigned l1HardwareResetDisable : 1;
  unsigned l2HardwareResetDisable : 1;
};

typedef union
{
  struct auxiliaryControlRegister bits;
  u32int value;
} ACTLR;


CREG *createCRB(void) __cold__;
u32int getCregVal(GCONTXT* context, u32int registerIndex);
void setCregVal(GCONTXT *context, u32int registerIndex, u32int value);


#endif /* __VM__OMAP_35XX__CP15_COPROC_H__ */
