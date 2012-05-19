#include "common/debug.h"
#include "common/stdlib.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/wdtimer.h"


// register offsets and bit values
#define WDT_WIDR            0x00
#define WDT_WD_SYSCONFIG    0x10
#define WDT_WD_SYSSTATUS    0x14
#define WDT_WISR            0x18
#define WDT_WIER            0x1C
#define WDT_WCLR            0x24
#define WDT_WCRR            0x28
#define WDT_WLDR            0x2C
#define WDT_WTGR            0x30
#define WDT_WWPS            0x34
#define WDT_WSPR            0x48


static void resetWatchdogTimer(struct WatchdogTimer *wt);


void initWDTimer2(virtualMachine *vm)
{
  // init function: setup device, reset register values to defaults!
  vm->wdtimer2 = (struct WatchdogTimer *)calloc(1, sizeof(struct WatchdogTimer));
  if (vm->wdtimer2 == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate MPU watchdog timer (WDT2).");
  }

  DEBUG(VP_OMAP_35XX_WDTIMER, "Initializing watchdog timer at %p" EOL, vm->wdtimer2);

  resetWatchdogTimer(vm->wdtimer2);
}

u32int loadWDTimer2(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  struct WatchdogTimer *const wt = context->vm.wdtimer2;
  u32int value = 0;
  u32int reg = phyAddr - WDTIMER2;

  switch (reg)
  {
    case WDT_WD_SYSCONFIG:
    {
      value = wt->wdSysconfig;
      break;
    }
    case WDT_WWPS:
    {
      value = wt->wwps;
      break;
    }
    default:
    {
      printf("%s: reg %#.8x addr %#.8x phy %#.8x" EOL, __func__, reg, virtAddr, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }

  DEBUG(VP_OMAP_35XX_WDTIMER, "%s: loading reg %x value %.8x" EOL, __func__, reg, value);

  return value;
}


void storeWDTimer2(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  struct WatchdogTimer *const wt = context->vm.wdtimer2;
  u32int reg = phyAddr - WDTIMER2;

  DEBUG(VP_OMAP_35XX_WDTIMER, "%s: storing reg %x value %.8x" EOL, __func__, reg, value);

  switch (reg)
  {
    case WDT_WD_SYSCONFIG:
    {
      wt->wdSysconfig = value;
      break;
    }
    case WDT_WSPR:
    {
      wt->wspr = value;
      break;
    }
    default:
    {
      printf("%s: reg %#.8x addr %#.8x phy %#.8x" EOL, __func__, reg, virtAddr, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

void resetWatchdogTimer(struct WatchdogTimer *wt)
{
  // reset to default register values
  wt->widr =        0x31;
  wt->wdSysconfig = 0;
  wt->wdSysstatus = 0x1;
  wt->wisr =        0;
  wt->wier =        0;
  wt->wclr =        0x20;
  wt->wcrr =        0;
  wt->wldr =        0xFFFB0000;
  wt->wtgr =        0;
  wt->wwps =        0;
  wt->wspr =        0;
}
