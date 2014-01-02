#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"
#ifdef CONFIG_PROFILER
#include "vm/omap35xx/profiler.h"
#endif

#include "guestManager/guestContext.h"

#include "vm/omap35xx/hardwareLibrary.h"
#ifndef CONFIG_HW_PASSTHROUGH
#include "vm/omap35xx/clockManager.h"
#include "vm/omap35xx/controlModule.h"
#include "vm/omap35xx/gpio.h"
#include "vm/omap35xx/gptimer.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/prm.h"
#include "vm/omap35xx/sdma.h"
#include "vm/omap35xx/sysControlModule.h"
#include "vm/omap35xx/timer32k.h"
#include "vm/omap35xx/wdtimer.h"
#ifdef CONFIG_MMC_GUEST_ACCESS
#include "vm/omap35xx/i2c.h"
#include "vm/omap35xx/mmc.h"
#endif
#endif
#include "vm/omap35xx/gpmc.h"
#include "vm/omap35xx/pm.h"
#include "vm/omap35xx/sdram.h"
#include "vm/omap35xx/sdrc.h"
#include "vm/omap35xx/sms.h"
#include "vm/omap35xx/sramInternal.h"
#include "vm/omap35xx/uart.h"


struct Device
{
  u32int startAddress;
  u32int endAddress;
  s16int left;
  s16int right;
  LOAD_FUNCTION load;
  STORE_FUNCTION store;
};


static const struct Device *findDevice(u32int physicalAddress);
static u32int loadFailure(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int physAddr);
static void storeFailure(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int physAddr, u32int value);


// Defines DEVICES and ROOT_DEVICE
#include "vm/omap35xx/omap3.vm.h"


void createHardwareLibrary(GCONTXT *context)
{
  initGpmc(&context->vm);
  initProtectionMechanism(&context->vm);
  initSms(&context->vm);
  initSdrc(&context->vm);

#ifndef CONFIG_HW_PASSTHROUGH
  initSysControlModule(&context->vm);
  initClockManager(&context->vm);
  initSdma(&context->vm);
#endif

  initUart(&context->vm, 1);
  initUart(&context->vm, 2);

#ifndef CONFIG_HW_PASSTHROUGH
#ifdef CONFIG_MMC_GUEST_ACCESS
  initI2c(&context->vm, 1);
  initI2c(&context->vm, 2);
  initI2c(&context->vm, 3);
  initVirtMmc(&context->vm, 1);
  initVirtMmc(&context->vm, 2);
  initVirtMmc(&context->vm, 3);
#endif
  initIntc(&context->vm);
  initPrm(&context->vm);
  initGpio(&context->vm, 1);
  initWDTimer2(&context->vm);
  initGPTimer(&context->vm);
  initTimer32k(&context->vm);
#endif

  initUart(&context->vm, 3);

#ifndef CONFIG_HW_PASSTHROUGH
  initGpio(&context->vm, 2);
  initGpio(&context->vm, 3);
  initGpio(&context->vm, 4);
  initGpio(&context->vm, 5);
  connectGpio(context->vm.gpio[5 - 1], 5);
  initGpio(&context->vm, 6);
#endif /* !CONFIG_HW_PASSTHROUGH */

  initSdram(&context->vm);
}

static const struct Device *findDevice(u32int physicalAddress)
{
  const struct Device *device = &DEVICES[ROOT_DEVICE];
  while (physicalAddress < device->startAddress || physicalAddress >= device->endAddress)
  {
    device += physicalAddress < device->startAddress ? device->left : device->right;
  }
  return device;
}

static u32int loadFailure(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int physAddr)
{
  printf("%s: VA=%.8x PA=%.8x" EOL, __func__, virtAddr, physAddr);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

static void storeFailure(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int physAddr, u32int value)
{
  printf("%s: VA=%.8x PA=%.8x value=%.8x" EOL, __func__, virtAddr, physAddr, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

u32int vmLoad(GCONTXT *gc, ACCESS_SIZE size, u32int virtAddr)
{
  u32int physAddr = getPhysicalAddress(gc, gc->virtAddrEnabled ? gc->pageTables->shadowActive : gc->hypervisorPageTable, virtAddr);
  return findDevice(physAddr)->load(gc, size, virtAddr, physAddr);
}

void vmStore(GCONTXT *gc, ACCESS_SIZE size, u32int virtAddr, u32int value)
{
  u32int physAddr = getPhysicalAddress(gc, gc->virtAddrEnabled ? gc->pageTables->shadowActive : gc->hypervisorPageTable, virtAddr);
  return findDevice(physAddr)->store(gc, size, virtAddr, physAddr, value);
}
