#ifndef __MEMORY_MANAGER__ADDRESSING_H__
#define __MEMORY_MANAGER__ADDRESSING_H__

#include "common/compiler.h"
#include "common/types.h"

#include "guestManager/guestContext.h"

#include "memoryManager/memoryProtection.h"


/* Need to initialise the MMU and enable virtual addressing */
void initVirtualAddressing(GCONTXT *context) __cold__;

u32int setProtection(u32int startAddr, u32int endAddr, u8int accessBits);

/* intercept new process page table creation & create shadow PT */
void guestSetPageTableBase(GCONTXT *gc, u32int ttbr);
void guestEnableMMU(GCONTXT *context);
void guestDisableMMU(GCONTXT *context);

void guestSetContextID(GCONTXT *context, u32int contextid);

void privToUserAddressing(GCONTXT *context);
void userToPrivAddressing(GCONTXT *context);

void initialiseShadowPageTables(GCONTXT *gc);

void changeGuestDACR(GCONTXT *context, DACR oldVal, DACR newVal);

void setExceptionVector(u32int guestMode);

#endif /* __MEMORY_MANAGER__ADDRESSING_H__ */
