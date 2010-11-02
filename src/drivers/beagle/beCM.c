#include "beCM.h"

#define CM_FCLKEN_DSS  0x48004E00
#define CM_ICLKEN_DSS  0x48004E10

#define readRegisterW(addr)          (*((volatile u32int *)(addr)))
#define writeRegisterW(addr, value)  { *((volatile u32int *)(addr)) = (value); }

void cmDisableDssClocks()
{
	writeRegisterW(CM_ICLKEN_DSS, 0);
	writeRegisterW(CM_FCLKEN_DSS, 0);
}

