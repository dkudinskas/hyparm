#include "common/debug.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/dmtimer.h"


/*
 * Henri
 * -----
 * This device does not actually exist in the OMAP35xx SoC, though linux 2.6.37
 * tries to initialize this device.
 * See: arch/arm/plat-omap/dmtimer.c
 *
 * Note 1: It seems this is the General-Purpose Timer 12 described in the
 *         OMAP35x manual spruf98d.pdf. GPT12 was later removed from the manual.
 *
 * Note 2: GP Timers are renamed to DM (Dual-Mode) Timers in OMAP4
 *
 * There's an option in the kernel config but disabling it breaks the build.
 */

#define DM_TIMER_RESET  0x48304014


u32int loadDmTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  printf("loadDMTimer: VA %08x PA %08x\n", virtAddr, phyAddr);
  DIE_NOW(context, "dmtimer should not be\n");
  return 0;
}

void storeDmTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  printf("storeDMTimer: VA %08x PA %08x val %08x\n", virtAddr, phyAddr, value);
  DIE_NOW(context, "dmtimer should not be\n");
}
