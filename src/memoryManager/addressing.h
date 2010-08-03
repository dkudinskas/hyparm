#ifndef __ADDRESSING_H__
#define __ADDRESSING_H__

#include "types.h"
#include "mmu.h"
#include "guestContext.h"

/* Un Comment me to define addressing debug */
// #define ADDR_DBG

/* Need to initialise the MMU and enable virtual addressing */
void initialiseVirtualAddressing(void);

u32int setProtection(u32int startAddr, u32int endAddr, u8int accessBits);

/* virtual machine startup */
void createVirtualMachineGPAtoRPA(GCONTXT* gc);

/* intercept new process page table creation & create shadow PT */
void initialiseGuestShadowPageTable(u32int guestPtAddr);
void guestEnableVirtMem(void);

#endif