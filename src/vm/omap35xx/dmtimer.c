#include "guestManager/guestContext.h"

#include "vm/omap35xx/dmtimer.h"


/*
 * Henri
 * -----
 * This device does not actually exist in the OMAP35xx SoC, though linux 2.6.37
 * tries to initialize this device.
 * See: arch/arm/plat-omap/dmtimer.c
 *
 * There's an option in the kernel config but disabling it breaks the build.
 */


u32int loadDmTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  return 0;
}

void storeDmTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  // ignored
}
