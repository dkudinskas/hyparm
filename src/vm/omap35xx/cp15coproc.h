#ifndef __MEMORY_MANAGER__CP15_COPROC_H__
#define __MEMORY_MANAGER__CP15_COPROC_H__

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


struct crbEntry
{
  u32int value;
  bool   valid;
};

typedef struct crbEntry CREG;

CREG *createCRB(void) __cold__;

u32int getCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr);
void   setCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr, u32int val);

#endif
