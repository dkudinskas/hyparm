#ifndef __CP15_COPROC_H__
#define __CP15_COPROC_H__

#include "types.h"
#include "serial.h"

// uncomment me for COPROCESSOR DEBUG: #define COPROC_DEBUG


#define MAX_CRN_VALUES    16
#define MAX_OPC1_VALUES   8
#define MAX_CRM_VALUES    16
#define MAX_OPC2_VALUES   8
#define MAX_CRB_SIZE      MAX_CRN_VALUES * MAX_OPC1_VALUES * MAX_CRM_VALUES * MAX_OPC2_VALUES

struct crbEntry
{
  u32int value;
  bool   valid;
};

typedef struct crbEntry CREG;

void initCRB(CREG * crb);

u32int getCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr);
void   setCregVal(u32int CRn, u32int opc1, u32int CRm, u32int opc2, CREG * crbPtr, u32int val);

u32int crbIndex(u32int CRn, u32int opc1, u32int CRm, u32int opc2);

#endif