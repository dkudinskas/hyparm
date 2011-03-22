#ifndef __MEMORY_MANAGER__ADDRESSING_H__
#define __MEMORY_MANAGER__ADDRESSING_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


//uncomment me to enable debugging: #define ADDRESSING_DEBUG

/* Need to initialise the MMU and enable virtual addressing */
void initialiseVirtualAddressing(void);

u32int setProtection(u32int startAddr, u32int endAddr, u8int accessBits);

/* virtual machine startup */
void createVirtualMachineGPAtoRPA(GCONTXT* gc);

/* intercept new process page table creation & create shadow PT */
void initialiseGuestShadowPageTable(u32int guestPtAddr);
void guestEnableVirtMem(void);

void changeGuestDomainAccessControl(u32int oldVal, u32int newVal);

#endif
