#ifndef __MEMORY_MANAGER__ADDRESSING_H__
#define __MEMORY_MANAGER__ADDRESSING_H__

#include "common/types.h"

#include "guestManager/guestContext.h"


//uncomment me to enable debugging: #define ADDRESSING_DEBUG

/* Need to initialise the MMU and enable virtual addressing */
void initVirtualAddressing(void);

void setupHypervisorPageTable(simpleEntry *pageTablePtr);
void setupShadowPageTable(simpleEntry *pageTablePtr);

u32int setProtection(u32int startAddr, u32int endAddr, u8int accessBits);

/* intercept new process page table creation & create shadow PT */
void guestSetPageTableBase(u32int ttbr);
void guestEnableMMU(void);
void guestDisableMMU(void);

void privToUserAddressing(void);
void userToPrivAddressing(void);

void initialiseShadowPageTables(GCONTXT *gc);

void changeGuestDACR(u32int oldVal, u32int newVal);
#endif
