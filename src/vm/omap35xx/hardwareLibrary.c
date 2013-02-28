#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"
#ifdef CONFIG_PROFILER
#include "vm/omap35xx/profiler.h"
#endif

#include "guestManager/guestContext.h"

#include "vm/omap35xx/clockManager.h"
#include "vm/omap35xx/gpio.h"
#include "vm/omap35xx/gpmc.h"
#include "vm/omap35xx/gptimer.h"
#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/prm.h"
#include "vm/omap35xx/sdma.h"
#include "vm/omap35xx/sdram.h"
#include "vm/omap35xx/sramInternal.h"
#include "vm/omap35xx/sysControlModule.h"
#include "vm/omap35xx/timer32k.h"
#include "vm/omap35xx/uart.h"
#include "vm/omap35xx/controlModule.h"
#ifdef CONFIG_MMC_GUEST_ACCESS
#include "vm/omap35xx/i2c.h"
#include "vm/omap35xx/mmc.h"
#endif
#include "vm/omap35xx/pm.h"
#include "vm/omap35xx/sdrc.h"
#include "vm/omap35xx/sms.h"
#include "vm/omap35xx/wdtimer.h"


static bool attachDevice(device *parent, device *child) __cold__;
static device *createDevice(const char *devName, bool isBus, u32int addrStart, u32int addrEnd,
                            device *parent, LOAD_FUNCTION ldFn, STORE_FUNCTION stFn) __cold__;
static inline bool isAddressInDevice(u32int address, device *dev);
static u32int loadGeneric(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int physAddr);
static void storeGeneric(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int physAddr, u32int value);


static bool attachDevice(device *parent, device *child)
{
  // only attach to 'bus' typed devices
  if (!parent->isBus)
  {
    return FALSE;
  }

  // check address range
  if ( (parent->startAddressMapped <= child->startAddressMapped) &&
       (parent->endAddressMapped >= child->endAddressMapped) )
  {
    child->parentDevice = parent;
    parent->attachedDevices[parent->nrOfAttachedDevs] = child;
    parent->nrOfAttachedDevs++;
    DEBUG(VP_OMAP_35XX_LIBRARY, "Attached %s to %s" EOL, child->deviceName, parent->deviceName);
    return TRUE;
  }
  return FALSE;
}

static device *createDevice(const char *devName, bool isBus, u32int addrStart, u32int addrEnd,
                            device *parent, LOAD_FUNCTION ldFn, STORE_FUNCTION stFn)
{
  DEBUG(VP_OMAP_35XX_LIBRARY, "Allocating device: %s" EOL, devName);

  device *dev = (device *)calloc(1, sizeof(device));
  if (dev == NULL)
  {
    printf("Error: allocation of device %s failed" EOL, devName);
    return NULL;
  }

  DEBUG(VP_OMAP_35XX_LIBRARY, "Initialising device: %s" EOL, devName);

  dev->deviceName = devName;
  dev->isBus = isBus;
  dev->startAddressMapped = addrStart;
  dev->endAddressMapped = addrEnd;
  dev->loadFunction = ldFn;
  dev->storeFunction = stFn;

  if (parent != NULL && !attachDevice(parent, dev))
  {
    // this is not the 'root' device, must be attached to something
    printf("Error: failed to attach device %s to device %s" EOL, devName, parent->deviceName);
    free(dev);
    return NULL;
  }

  return dev;
}

device *createHardwareLibrary(GCONTXT *context)
{
  /*
   * Here we allocate all device structures. If any allocation fails, deallocation happens in
   * reverse order and the function returns NULL.
   */
  DEBUG(VP_OMAP_35XX_LIBRARY, "Allocating hardware library structures..." EOL);

  // top level device, everything hangs on it
  device *topLevelBus = createDevice("TopLevelBus", TRUE, 0, 0xFFFFFFFF, 0, &loadGeneric,
                                     &storeGeneric);
  if (topLevelBus == NULL)
  {
    goto topLevelBusError;
  }

  // QUARTER 0
  device *q0bus = createDevice("Q0Bus", TRUE, QUARTER0, (u32int)(QUARTER0 + QUARTER_SIZE - 1),
                               topLevelBus, &loadGeneric, &storeGeneric);
  if (q0bus == NULL)
  {
    goto q0busError;
  }

  // QUARTER 1
  device *q1bus = createDevice("Q1Bus", TRUE, QUARTER1, (u32int)(QUARTER1 - 1 + QUARTER_SIZE),
                               topLevelBus, &loadGeneric, &storeGeneric);
  if (q1bus == NULL)
  {
    goto q1busError;
  }

  // Q1: ON CHIP MEMORY (OCM, parent Q1)
  device *onChipMemory = createDevice("ON_CHIP_MEMORY", TRUE, Q1_ON_CHIP_MEMORY,
                                      (u32int)(Q1_ON_CHIP_MEMORY + Q1_ON_CHIP_MEMORY_SIZE - 1),
                                      q1bus, &loadGeneric, &storeGeneric);
  if (onChipMemory == NULL)
  {
    goto onChipMemoryError;
  }

  // OCM: secure boot rom (parent ocm)
  device *bootRomSecure = createDevice("BOOT_ROM_SECURE", FALSE, Q1_OCM_BOOT_ROM_SECURE,
                                       (u32int)(Q1_OCM_BOOT_ROM_SECURE + Q1_OCM_BOOT_ROM_SECURE_SIZE - 1),
                                       onChipMemory, &loadGeneric, &storeGeneric);
  if (bootRomSecure == NULL)
  {
    goto bootRomSecureError;
  }

  // OCM: public boot rom
  device *bootRomPublic = createDevice("BOOT_ROM_PUBLIC", FALSE, Q1_OCM_BOOT_ROM_PUBLIC,
                                       (u32int)(Q1_OCM_BOOT_ROM_PUBLIC + Q1_OCM_BOOT_ROM_PUBLIC_SIZE - 1),
                                       onChipMemory, &loadGeneric, &storeGeneric);
  if (bootRomPublic == NULL)
  {
    goto bootRomPublicError;
  }

  // OCM: internal sram
  device *sramInternal = createDevice("SRAM_INTERNAL", FALSE, Q1_OCM_SRAM_INTERNAL,
                                      (u32int)(Q1_OCM_SRAM_INTERNAL + Q1_OCM_SRAM_INTERNAL_SIZE - 1),
                                      onChipMemory, &loadSramInternal, &storeSramInternal);
  if (sramInternal == NULL)
  {
    goto sramInternalError;
  }

  // Q1: LEVEL3 INTERCONNECT (L3INT, parent Q1)
  device *l3Interconnect = createDevice("L3_INTERCONNECT", TRUE, Q1_L3_INTERCONNECT,
                                        (u32int)(Q1_L3_INTERCONNECT + Q1_L3_INTERCONNECT_SIZE - 1),
                                        q1bus, &loadGeneric, &storeGeneric);
  if (l3Interconnect == NULL)
  {
    goto l3InterconnectError;
  }

  // L3INT: general purpose memory controller
  device *gpmcModule = createDevice("GPMC", FALSE, Q1_L3_GPMC,
                                    (u32int)(Q1_L3_GPMC - 1 + Q1_L3_GPMC_SIZE), l3Interconnect,
                                    &loadGpmc, &storeGpmc);
  if (gpmcModule == NULL)
  {
    goto gpmcModuleError;
  }
  initGpmc(&context->vm);

  // L3INT: Protection Mechanism (PM)
  device *pmModule = createDevice("L3_PM", FALSE, Q1_L3_PM, (u32int)(Q1_L3_PM + Q1_L3_PM_SIZE - 1),
                                  l3Interconnect, &loadProtectionMechanism,
                                  &storeProtectionMechanism);
  if (pmModule == NULL)
  {
    goto pmModuleError;
  }
  initProtectionMechanism(&context->vm);

  // L3INT: SDRAM Memory Scheduler
  device *smsModule = createDevice("L3_SMS", FALSE,
                                   Q1_L3_SMS, (u32int)(Q1_L3_SMS + Q1_L3_SMS_SIZE - 1),
                                   l3Interconnect, &loadSms, &storeSms);
  if (smsModule == NULL)
  {
    goto smsModuleError;
  }
  initSms(&context->vm);

  // L3INT: SDRAM Controller subsystem
  device *sdrcModule = createDevice("L3_SDRC", FALSE,
                                   Q1_L3_SDRC, (u32int)(Q1_L3_SDRC + Q1_L3_SDRC_SIZE - 1),
                                   l3Interconnect, &loadSdrc, &storeSdrc);
  if (sdrcModule == NULL)
  {
    goto sdrcModuleError;
  }
  initSdrc(&context->vm);

  // Q1: LEVEL4 INTERCONNECT (L4INT, parent Q1)
  device *l4Interconnect = createDevice("L4_INTERCONNECT", TRUE, Q1_L4_INTERCONNECT,
                                        (u32int)(Q1_L4_INTERCONNECT + Q1_L4_INTERCONNECT_SIZE - 1),
                                        q1bus, &loadGeneric, &storeGeneric);
  if (l4Interconnect == NULL)
  {
    goto l4InterconnectError;
  }

  // L4INT: core
  device *l4IntCore = createDevice("L4_INTERCONNECT_CORE", TRUE, Q1_L4_INT_CORE,
                                   (u32int)(Q1_L4_INT_CORE - 1 + Q1_L4_INT_CORE_SIZE),
                                   l4Interconnect, &loadGeneric, &storeGeneric);
  if (l4IntCore == NULL)
  {
    goto l4IntCoreError;
  }

  // L4INT_CORE: system control module
  device *sysCtrlMod = createDevice("SYSCTRL_MOD", FALSE, SYS_CONTROL_MODULE,
                                    (u32int)(SYS_CONTROL_MODULE - 1 + SYS_CONTROL_MODULE_SIZE),
                                    l4IntCore, &loadSysCtrlModule, &storeSysCtrlModule);
  if (sysCtrlMod == NULL)
  {
    goto sysCtrlModError;
  }
  initSysControlModule(&context->vm);

  // L4INT_CORE: clock manager (and DPLL)
  device *clockManager = createDevice("CLOCK_MAN", FALSE, CLOCK_MANAGER,
                                      (u32int)(CLOCK_MANAGER - 1 + CLOCK_MANAGER_SIZE), l4IntCore,
                                      &loadClockManager, &storeClockManager);
  if (clockManager == NULL)
  {
    goto clockManagerError;
  }
  initClockManager(&context->vm);

  // L4INT_CORE: SDMA
  device *sdmaModule = createDevice("SDMA", FALSE, SDMA, (u32int) (SDMA - 1 + SDMA_SIZE),
                                    l4IntCore, &loadSdma, &storeSdma);
  if (sdmaModule == NULL)
  {
    goto sdmaModuleError;
  }
  initSdma(&context->vm);

  // L4INT_CORE: uart1
  device *uart1 = createDevice("UART1", FALSE, UART1, (u32int) (UART1 - 1 + UART1_SIZE), l4IntCore,
                               &loadUart, &storeUart);
  if (uart1 == NULL)
  {
    goto uart1Error;
  }
  initUart(&context->vm, 1);

  // L4INT_CORE: uart2
  device *uart2 = createDevice("UART2", FALSE, UART2, (u32int) (UART2 - 1 + UART2_SIZE), l4IntCore,
                               &loadUart, &storeUart);
  if (uart2 == NULL)
  {
    goto uart2Error;
  }
  initUart(&context->vm, 2);

#ifdef CONFIG_MMC_GUEST_ACCESS
  // I2C1
  device * i2c1 = createDevice("I2C1", FALSE, I2C1, (u32int) (I2C1 - 1 + I2C1_SIZE), l4IntCore,
                               &loadI2c, &storeI2c);
  if (i2c1 == NULL)
  {
    goto i2c1Error;
  }
  initI2c(&context->vm, 1);
  
  // I2C2
  device * i2c2 = createDevice("I2C2", FALSE, I2C2, (u32int) (I2C2 - 1 + I2C2_SIZE), l4IntCore,
                               &loadI2c, &storeI2c);
  if (i2c2 == NULL)
  {
    goto i2c2Error;
  }
  initI2c(&context->vm, 2);
  
  // I2C3
  device * i2c3 = createDevice("I2C3", FALSE, I2C3, (u32int) (I2C3 - 1 + I2C3_SIZE), l4IntCore,
                               &loadI2c, &storeI2c);
  if (i2c3 == NULL)
  {
    goto i2c3Error;
  }
  initI2c(&context->vm, 3);

  // L4INT_CORE: MMC/SD/SDIO1
  device *mmc1 = createDevice("SD_MMC1", FALSE, SD_MMC1, (u32int) (SD_MMC1 - 1 + SD_MMC1_SIZE), l4IntCore,
                               &loadMmc, &storeMmc);
  if (mmc1 == NULL)
  {
    goto mmc1Error;
  }
  initVirtMmc(&context->vm, 1);

  // L4INT_CORE: MMC/SD/SDIO2
  device *mmc2 = createDevice("SD_MMC2", FALSE, SD_MMC2, (u32int) (SD_MMC2 - 1 + SD_MMC2_SIZE), l4IntCore,
                               &loadMmc, &storeMmc);
  if (mmc2 == NULL)
  {
    goto mmc2Error;
  }
  initVirtMmc(&context->vm, 2);

  // L4INT_CORE: MMC/SD/SDIO3
  device *mmc3 = createDevice("SD_MMC1", FALSE, SD_MMC3, (u32int) (SD_MMC3 - 1 + SD_MMC3_SIZE), l4IntCore,
                               &loadMmc, &storeMmc);
  if (mmc3 == NULL)
  {
    goto mmc3Error;
  }
  initVirtMmc(&context->vm, 3);
#endif 

  // L4INT_CORE: interrupt controller
  device *intc = createDevice("INTC", FALSE, INTERRUPT_CONTROLLER,
                              (u32int)(INTERRUPT_CONTROLLER - 1 + INTERRUPT_CONTROLLER_SIZE),
                              l4IntCore, &loadIntc, &storeIntc);
  if (intc == NULL)
  {
    goto intcError;
  }
  initIntc(&context->vm);

  // L4INT_CORE: core wakeup interconnect
  device *l4CoreWakeupInt = createDevice("L4_CORE_WAKEUP_INT", TRUE, L4_CORE_WAKEUP_INT,
                                         (u32int)(L4_CORE_WAKEUP_INT - 1 + L4_CORE_WAKEUP_INT_SIZE),
                                         l4IntCore, &loadGeneric, &storeGeneric);
  if (l4CoreWakeupInt == NULL)
  {
    goto l4CoreWakeupIntError;
  }


  // L4_CORE_WAKEUP: power and reset manager
  device *prm = createDevice("PRM", FALSE, PRM, (u32int)(PRM - 1 + PRM_SIZE), l4CoreWakeupInt,
                             &loadPrm, &storePrm);
  if (prm == NULL)
  {
    goto prmError;
  }
  initPrm(&context->vm);

  // L4_CORE_CONTROL: control module ID
  device * ctrlModID = createDevice("CONTROL_MODULE_ID", FALSE, CONTROL_MODULE_ID,
                                    (u32int)(CONTROL_MODULE_ID - 1 + CONTROL_MODULE_ID_SIZE),
                                    l4CoreWakeupInt, &loadControlModule, &storeControlModule);
  if (ctrlModID == 0)
  {
    goto ctrlModIDError;
  }

  // L4_CORE_WAKEUP: general purpose I/O 1
  device *gpio1 = createDevice("GPIO1", FALSE, GPIO1, (u32int) (GPIO1 - 1 + GPIO1_SIZE),
                               l4CoreWakeupInt, &loadGpio, &storeGpio);
  if (gpio1 == NULL)
  {
    goto gpio1Error;
  }
  initGpio(&context->vm, 1);

  // L4_CORE_WAKEUP: watchdog timer 2
  device *wdtimer2 = createDevice("WDTIMER2", FALSE, WDTIMER2, (u32int)(WDTIMER2 - 1 + WDTIMER2_SIZE),
                                  l4CoreWakeupInt, &loadWDTimer2, &storeWDTimer2);
  if (wdtimer2 == NULL)
  {
    goto wdtimer2Error;
  }
  initWDTimer2(&context->vm);

  // L4_CORE_WAKEUP: general purpose timer 1
  device *gptimer1 = createDevice("GPTIMER1", FALSE, GPTIMER1, (u32int)(GPTIMER1 - 1 + GPTIMER1_SIZE),
                                  l4CoreWakeupInt, &loadGPTimer, &storeGPTimer);
  if (gptimer1 == NULL)
  {
    goto gptimer1Error;
  }
  initGPTimer(&context->vm);

  // L4_CORE_WAKEUP: 32 Kiloherz synchronised timer
  device *timer32k = createDevice("32kTIMER", FALSE, TIMER_32K,
                                  (u32int)(TIMER_32K - 1 + TIMER_32K_SIZE), l4CoreWakeupInt,
                                  &loadTimer32k, &storeTimer32k);
  if (timer32k == NULL)
  {
    goto timer32kError;
  }
  initTimer32k(&context->vm);

  // L4 interconnect: L4 interconnect peripherals
  device *l4IntPer = createDevice("L4_INT_PER", TRUE, Q1_L4_INT_PER,
                                  (u32int)(Q1_L4_INT_PER - 1 + Q1_L4_INT_PER_SIZE), l4Interconnect,
                                  &loadGeneric, &storeGeneric);
  if (l4IntPer == NULL)
  {
    goto l4IntPerError;
  }

  // L4INT_PER: uart3
  device *uart3 = createDevice("UART3", FALSE, UART3, (u32int)(UART3 - 1 + UART3_SIZE), l4IntPer,
                               &loadUart, &storeUart);
  if (uart3 == NULL)
  {
    goto uart3Error;
  }
  initUart(&context->vm, 3);

  // L4_INT_PER: general purpose I/O 2
  device *gpio2 = createDevice("GPIO2", FALSE, GPIO2, (u32int)(GPIO2 - 1 + GPIO2_SIZE), l4IntPer,
                               &loadGpio, &storeGpio);
  if (gpio2 == NULL)
  {
    goto gpio2Error;
  }
  initGpio(&context->vm, 2);

  // L4_INT_PER: general purpose I/O 3
  device *gpio3 = createDevice("GPIO3", FALSE, GPIO3, (u32int)(GPIO3 - 1 + GPIO3_SIZE), l4IntPer,
                               &loadGpio, &storeGpio);
  if (gpio3 == NULL)
  {
    goto gpio3Error;
  }
  initGpio(&context->vm, 3);

  // L4_INT_PER: general purpose I/O 4
  device *gpio4 = createDevice("GPIO4", FALSE, GPIO4, (u32int)(GPIO4 - 1 + GPIO4_SIZE), l4IntPer,
                               &loadGpio, &storeGpio);
  if (gpio4 == NULL)
  {
    goto gpio4Error;
  }
  initGpio(&context->vm, 4);

  // L4_INT_PER: general purpose I/O 5
  device *gpio5 = createDevice("GPIO5", FALSE, GPIO5, (u32int)(GPIO5 - 1 + GPIO5_SIZE), l4IntPer,
                               &loadGpio, &storeGpio);
  if (gpio5 == NULL)
  {
    goto gpio5Error;
  }
  initGpio(&context->vm, 5);
  connectGpio(context->vm.gpio[5 - 1], 5);

  // L4_INT_PER: general purpose I/O 6
  device *gpio6 = createDevice("GPIO6", FALSE, GPIO6, (u32int)(GPIO6 - 1 + GPIO6_SIZE), l4IntPer,
                               &loadGpio, &storeGpio);
  if (gpio6 == NULL)
  {
    goto gpio6Error;
  }
  initGpio(&context->vm, 6);

  // QUARTER 2
  device *q2bus = createDevice("Q2Bus", TRUE, QUARTER2, (u32int) (QUARTER2 - 1 + QUARTER_SIZE),
                               topLevelBus, &loadGeneric, &storeGeneric);
  if (q2bus == NULL)
  {
    goto q2busError;
  }

  // Q2: sdram
  device *sdramModule = createDevice("SDRAM", FALSE, Q2_SDRC_SMS,
                                     (u32int)(Q2_SDRC_SMS + Q2_SDRC_SMS_SIZE - 1), q2bus,
                                     &loadSdram, &storeSdram);
  if (sdramModule == NULL)
  {
    goto sdramModuleError;
  }
  initSdram(&context->vm);

  // QUARTER 3
  device *q3bus = createDevice("Q3Bus", TRUE, QUARTER3, (u32int)(QUARTER3 - 1 + QUARTER_SIZE),
                               topLevelBus, &loadGeneric, &storeGeneric);
  if (q3bus == NULL)
  {
    goto q3busError;
  }

  
#ifdef CONFIG_PROFILER
  device *profiler = createDevice("Profiler", FALSE, PROFILER, 
                                  (u32int)(PROFILER - 1 + PROFILER_SIZE), l4IntCore,
                                  &loadProfilerInt, &storeProfilerInt);
  if (profiler == NULL)
  {
    goto profilerError;
  }
#endif
  return topLevelBus;

  /*
   * Deallocation in reverse order to make sure no uninitialized pointer values are freed.
   * WARNING: this is required because pointers are NOT guaranteed to be NULL if uninitialized.
   */
#ifdef CONFIG_PROFILER
  free(profiler);
profilerError:
#endif
  free(q3bus);
q3busError:
  free(sdramModule);
sdramModuleError:
  free(q2bus);
q2busError:
  free(gpio6);
gpio6Error:
  free(gpio5);
gpio5Error:
  free(gpio4);
gpio4Error:
  free(gpio3);
gpio3Error:
  free(gpio2);
gpio2Error:
  free(uart3);
uart3Error:
  free(l4IntPer);
l4IntPerError:
  free(timer32k);
timer32kError:
  free(gptimer1);
gptimer1Error:
  free(wdtimer2);
wdtimer2Error:
  free(gpio1);
gpio1Error:
  free(ctrlModID);
ctrlModIDError:
  free(prm);
prmError:
  free(l4CoreWakeupInt);
l4CoreWakeupIntError:
  free(intc);
intcError:
#ifdef CONFIG_MMC_GUEST_ACCESS
  free(mmc3);
mmc3Error:
  free(mmc2);
mmc2Error:
  free(mmc1);
mmc1Error:
  free(i2c3);
i2c3Error:
  free(i2c2);
i2c2Error:
  free(i2c1);
i2c1Error:
  free(uart2);
#endif
uart2Error:
  free(uart1);
uart1Error:
  free(sdmaModule);
sdmaModuleError:
  free(clockManager);
clockManagerError:
  free(sysCtrlMod);
sysCtrlModError:
  free(l4IntCore);
l4IntCoreError:
  free(l4Interconnect);
l4InterconnectError:
  free(sdrcModule);
sdrcModuleError:
  free(smsModule);
smsModuleError:
  free(pmModule);
pmModuleError:
  free(gpmcModule);
gpmcModuleError:
  free(l3Interconnect);
l3InterconnectError:
  free(sramInternal);
sramInternalError:
  free(bootRomPublic);
bootRomPublicError:
  free(bootRomSecure);
bootRomSecureError:
  free(onChipMemory);
onChipMemoryError:
  free(q1bus);
q1busError:
  free(q0bus);
q0busError:
  free(topLevelBus);
topLevelBusError:
  return NULL;
}


static inline bool isAddressInDevice(u32int address, device *dev)
{
  return (address >= dev->startAddressMapped) && (address <= dev->endAddressMapped);
}


/**************************************
 * generic LOAD/STORE functions       *
 **************************************/
static u32int loadGeneric(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  if (dev->isBus)
  {
    // bus device
    u32int index;
    for (index = 0; index < dev->nrOfAttachedDevs; index++)
    {
      if (isAddressInDevice(phyAddr, dev->attachedDevices[index]))
      {
        // hit the address range!
        return dev->attachedDevices[index]->loadFunction(context, dev->attachedDevices[index], size, virtAddr, phyAddr);
      }
    }
    printf("Load from %s address %.8x physical %.8x" EOL, dev->deviceName, virtAddr, phyAddr);
    DIE_NOW(NULL, "No child of current device holds load address in range.");
  }
  else
  {
    // not a bus, end device
    printf("Load from %s address %.8x physical %.8x" EOL, dev->deviceName, virtAddr, phyAddr);
    DIE_NOW(NULL, "End device didn't implement custom load function!");
  }

  return 0;
}

static void storeGeneric(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  if (dev->isBus)
  {
    // bus device
    u32int index;
    for (index = 0; index < dev->nrOfAttachedDevs; index++)
    {
      if (isAddressInDevice(phyAddr, dev->attachedDevices[index]))
      {
        // hit the address range!
        dev->attachedDevices[index]->storeFunction(context, dev->attachedDevices[index], size, virtAddr, phyAddr, value);
        return;
      }
    }
    printf("Store to %s at address %.8x physical %.8x value %.8x" EOL, dev->deviceName, virtAddr, phyAddr, value);
    DIE_NOW(NULL, "No child of current device holds store address in range.");
  }
  else
  {
    // not a bus, end device
    printf("Store to %s at address %.8x physical %.8x value %.8x" EOL,
           dev->deviceName, virtAddr, phyAddr, value);
    DIE_NOW(NULL, "End device didn't implement custom store function!");
  }
}

u32int vmLoad(GCONTXT *gc, ACCESS_SIZE size, u32int virtAddr)
{
  u32int physAddr = getPhysicalAddress(gc, gc->virtAddrEnabled ? gc->pageTables->shadowActive : gc->pageTables->hypervisor, virtAddr);
  return gc->hardwareLibrary->loadFunction(gc, gc->hardwareLibrary, size, virtAddr, physAddr);
}

void vmStore(GCONTXT *gc, ACCESS_SIZE size, u32int virtAddr, u32int value)
{
  u32int physAddr = getPhysicalAddress(gc, gc->virtAddrEnabled ? gc->pageTables->shadowActive : gc->pageTables->hypervisor, virtAddr);
  gc->hardwareLibrary->storeFunction(gc, gc->hardwareLibrary, size, virtAddr, physAddr, value);
}
