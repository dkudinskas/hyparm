#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beIntc.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/clockManager.h"
#include "vm/omap35xx/wdtimer.h"


struct WatchdogTimer *wdtimer2;


void initWDTimer2()
{
  // init function: setup device, reset register values to defaults!
  wdtimer2 = (struct WatchdogTimer *)calloc(1, sizeof(struct WatchdogTimer));
  if (wdtimer2 == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate MPU watchdog timer (WDT2).");
  }

  DEBUG(VP_OMAP_35XX_WDTIMER, "Initializing watchdog timer at %p" EOL, wdtimer2);

  resetWDTimer2();
}

void resetWDTimer2(void)
{
  // reset to default register values
  wdtimer2->widr =        0x31;
  wdtimer2->wdSysconfig = 0;
  wdtimer2->wdSysstatus = 0x1;
  wdtimer2->wisr =        0;
  wdtimer2->wier =        0;
  wdtimer2->wclr =        0x20;
  wdtimer2->wcrr =        0;
  wdtimer2->wldr =        0xFFFB0000;
  wdtimer2->wtgr =        0;
  wdtimer2->wwps =        0;
  wdtimer2->wspr =        0;
}


u32int loadWDTimer2(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int value = 0;
  u32int reg = phyAddr - WDTIMER2;

  switch (reg)
  {
    case WDT_WD_SYSCONFIG:
    {
      value = wdtimer2->wdSysconfig;
      break;
    }
    case WDT_WWPS:
    {
      value = wdtimer2->wwps;
      break;
    }
    default:
    {
      printf("%s: reg %#.8x addr %#.8x phy %#.8x" EOL, __func__, reg, virtAddr, phyAddr);
      DIE_NOW(NULL, "WDT2: load from undefined register.");
    }
  }

  DEBUG(VP_OMAP_35XX_WDTIMER, "%s: loading reg %x value %.8x" EOL, __func__, reg, value);

  return value;
}


void storeWDTimer2(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  u32int reg = phyAddr - WDTIMER2;

  DEBUG(VP_OMAP_35XX_WDTIMER, "%s: storing reg %x value %.8x" EOL, __func__, reg, value);

  switch (reg)
  {
    case WDT_WD_SYSCONFIG:
    {
      wdtimer2->wdSysconfig = value;
      break;
    }
    case WDT_WSPR:
    {
      wdtimer2->wspr = value;
      break;
    }
    default:
    {
      printf("%s: reg %#.8x addr %#.8x phy %#.8x" EOL, __func__, reg, virtAddr, phyAddr);
      DIE_NOW(NULL, "WDT: store to undefined register.");
    }
  }
}

