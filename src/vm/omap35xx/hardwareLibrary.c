#include "common/debug.h"
#include "common/memFunctions.h"

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


extern GCONTXT * getGuestContext(void);

device * initialiseHardwareLibrary()
{
#ifdef HARDWARE_LIB_DBG
  printf("Initialising device library...\n");
#endif

  // top level device, everything hangs on it
  device * topLevelBus = (device*)mallocBytes(sizeof(device));
  if (topLevelBus == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate top level bus.\n");
  }
  else
  {
    memset((void*)topLevelBus, 0x0, sizeof(device));
  }
  initialiseDevice(topLevelBus, "TopLevelBus", TRUE, 0, 0xFFFFFFFF,
                   0, &loadGeneric, &storeGeneric);


  // QUARTER 0
  device * q0bus = (device*)mallocBytes(sizeof(device));
  if (q0bus == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate Q0 bus.\n");
  }
  else
  {
    memset((void*)q0bus, 0x0, sizeof(device));
  }
  initialiseDevice(q0bus, "Q0Bus", TRUE, QUARTER0, (u32int)(QUARTER0+QUARTER_SIZE-1),
                   topLevelBus, &loadGeneric, &storeGeneric);


  // QUARTER 1
  device * q1bus = (device*)mallocBytes(sizeof(device));
  if (q1bus == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate Q1 bus.\n");
  }
  else
  {
    memset((void*)q1bus, 0x0, sizeof(device));
  }
  initialiseDevice(q1bus, "Q1Bus", TRUE, QUARTER1, (u32int)(QUARTER1-1+QUARTER_SIZE),
                   topLevelBus, &loadGeneric, &storeGeneric);


  // Q1: ON CHIP MEMORY (OCM, parent Q1)
  device * onChipMemory = (device*)mallocBytes(sizeof(device));
  if (onChipMemory == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate on chip memory.\n");
  }
  else
  {
    memset((void*)onChipMemory, 0x0, sizeof(device));
  }
  initialiseDevice(onChipMemory, "ON_CHIP_MEMORY", TRUE,
                   Q1_ON_CHIP_MEMORY, (u32int)(Q1_ON_CHIP_MEMORY+Q1_ON_CHIP_MEMORY_SIZE-1),
                   q1bus, &loadGeneric, &storeGeneric);


  // OCM: secure boot rom (parent ocm)
  device * bootRomSecure = (device*)mallocBytes(sizeof(device));
  if (bootRomSecure == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate secure boot rom.\n");
  }
  else
  {
    memset((void*)bootRomSecure, 0x0, sizeof(device));
  }
  initialiseDevice(bootRomSecure, "BOOT_ROM_SECURE", FALSE,
              Q1_OCM_BOOT_ROM_SECURE, (u32int)(Q1_OCM_BOOT_ROM_SECURE+Q1_OCM_BOOT_ROM_SECURE_SIZE-1),
              onChipMemory, &loadGeneric, &storeGeneric);


  // OCM: public boot rom
  device * bootRomPublic = (device*)mallocBytes(sizeof(device));
  if (bootRomPublic == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate public boot rom.\n");
  }
  else
  {
    memset((void*)bootRomPublic, 0x0, sizeof(device));
  }
  initialiseDevice(bootRomPublic, "BOOT_ROM_PUBLIC", FALSE,
              Q1_OCM_BOOT_ROM_PUBLIC, (u32int)(Q1_OCM_BOOT_ROM_PUBLIC+Q1_OCM_BOOT_ROM_PUBLIC_SIZE-1),
              onChipMemory, &loadGeneric, &storeGeneric);


  // OCM: internal sram
  device * sramInternal = (device*)mallocBytes(sizeof(device));
  if (sramInternal == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate internal SRAM.\n");
  }
  else
  {
    memset((void*)sramInternal, 0x0, sizeof(device));
  }
  initialiseDevice(sramInternal, "SRAM_INTERNAL", FALSE,
              Q1_OCM_SRAM_INTERNAL, (u32int)(Q1_OCM_SRAM_INTERNAL+Q1_OCM_SRAM_INTERNAL_SIZE-1),
              onChipMemory, &loadSramInternal, &storeSramInternal);


  // Q1: LEVEL3 INTERCONNECT (L3INT, parent Q1)
  device * l3Interconnect = (device*)mallocBytes(sizeof(device));
  if (l3Interconnect == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate L3 interconnect bus.\n");
  }
  else
  {
    memset((void*)l3Interconnect, 0x0, sizeof(device));
  }
  initialiseDevice(l3Interconnect, "L3_INTERCONNECT", TRUE,
                   Q1_L3_INTERCONNECT, (u32int)(Q1_L3_INTERCONNECT+Q1_L3_INTERCONNECT_SIZE-1),
                   q1bus, &loadGeneric, &storeGeneric);


  // L3INT: general purpose memory controller
  device * gpmcModule = (device*)mallocBytes(sizeof(device));
  if (gpmcModule == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPMC.\n");
  }
  else
  {
    memset((void*)gpmcModule, 0x0, sizeof(device));
  }
  initGpmc();
  initialiseDevice(gpmcModule, "GPMC", FALSE,
                   Q1_L3_GPMC, (u32int)(Q1_L3_GPMC-1+Q1_L3_GPMC_SIZE),
                   l3Interconnect, &loadGpmc, &storeGpmc);


  // Q1: LEVEL4 INTERCONNECT (L4INT, parent Q1)
  device * l4Interconnect = (device*)mallocBytes(sizeof(device));
  if (l4Interconnect == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate L4 Interconnect bus.\n");
  }
  else
  {
    memset((void*)l4Interconnect, 0x0, sizeof(device));
  }
  initialiseDevice(l4Interconnect, "L4_INTERCONNECT", TRUE,
                   Q1_L4_INTERCONNECT, (u32int)(Q1_L4_INTERCONNECT+Q1_L4_INTERCONNECT_SIZE-1),
                   q1bus, &loadGeneric, &storeGeneric);


  // L4INT: core
  device * l4IntCore = (device*)mallocBytes(sizeof(device));
  if (l4IntCore == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate L4 Interconnect core.\n");
  }
  else
  {
    memset((void*)l4IntCore, 0x0, sizeof(device));
  }
  initialiseDevice(l4IntCore, "L4_INTERCONNECT_CORE", TRUE,
                   Q1_L4_INT_CORE, (u32int)(Q1_L4_INT_CORE-1+Q1_L4_INT_CORE_SIZE),
                   l4Interconnect, &loadGeneric, &storeGeneric);


  // L4INT_CORE: system control module
  device * sysCtrlMod = (device*)mallocBytes(sizeof(device));
  if (sysCtrlMod == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate System control module.\n");
  }
  else
  {
    memset((void*)sysCtrlMod, 0x0, sizeof(device));
  }
  initSysControlModule();
  initialiseDevice(sysCtrlMod, "SYSCTRL_MOD", FALSE,
                   SYS_CONTROL_MODULE, (u32int)(SYS_CONTROL_MODULE -1 + SYS_CONTROL_MODULE_SIZE),
                   l4IntCore, &loadSysCtrlModule, &storeSysCtrlModule);


  // L4INT_CORE: clock manager (and DPLL)
  device * clockManager = (device*)mallocBytes(sizeof(device));
  if (clockManager == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate Clock Manager module.\n");
  }
  else
  {
    memset((void*)clockManager, 0x0, sizeof(device));
  }
  initClockManager();
  initialiseDevice(clockManager, "CLOCK_MAN", FALSE,
                   CLOCK_MANAGER, (u32int)(CLOCK_MANAGER -1 + CLOCK_MANAGER_SIZE),
                   l4IntCore, &loadClockManager, &storeClockManager);


  // L4INT_CORE: SDMA
  device * sdmaModule = (device*)mallocBytes(sizeof(device));
  if (sdmaModule == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate SDMA.\n");
  }
  else
  {
    memset((void*)sdmaModule, 0x0, sizeof(device));
  }
  initSdma();
  initialiseDevice(sdmaModule, "SDMA", FALSE,
                   SDMA, (u32int)(SDMA -1 + SDMA_SIZE),
                   l4IntCore, &loadSdma, &storeSdma);


  // L4INT_CORE: uart1
  device * uart1 = (device*)mallocBytes(sizeof(device));
  if (uart1 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate UART1.\n");
  }
  else
  {
    memset((void*)uart1, 0x0, sizeof(device));
  }
  initUart(1);
  initialiseDevice(uart1, "UART1", FALSE,
                   UART1, (u32int)(UART1 -1 + UART1_SIZE),
                   l4IntCore, &loadUart, &storeUart);


  // L4INT_CORE: uart2
  device * uart2 = (device*)mallocBytes(sizeof(device));
  if (uart2 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate UART2.\n");
  }
  else
  {
    memset((void*)uart2, 0x0, sizeof(device));
  }
  initUart(2);
  initialiseDevice(uart2, "UART2", FALSE,
                   UART2, (u32int)(UART2 - 1 + UART2_SIZE),
                   l4IntCore, &loadUart, &storeUart);


  // L4INT_CORE: interrupt controller
  device * intc = (device*)mallocBytes(sizeof(device));
  if (intc == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate interrupt controller.\n");
  }
  else
  {
    memset((void*)intc, 0x0, sizeof(device));
  }
  initIntc();
  initialiseDevice(intc, "INTC", FALSE,
                   INTERRUPT_CONTROLLER, (u32int)(INTERRUPT_CONTROLLER -1 + INTERRUPT_CONTROLLER_SIZE),
                   l4IntCore, &loadIntc, &storeIntc);


  // L4INT_CORE: core wakeup interconnect
  device * l4CoreWakeupInt = (device*)mallocBytes(sizeof(device));
  if (l4CoreWakeupInt == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate L4 Core Wakeup interconnect.\n");
  }
  else
  {
    memset((void*)l4CoreWakeupInt, 0x0, sizeof(device));
  }
  initialiseDevice(l4CoreWakeupInt, "L4_CORE_WAKEUP_INT", TRUE,
                   L4_CORE_WAKEUP_INT, (u32int)(L4_CORE_WAKEUP_INT-1+L4_CORE_WAKEUP_INT_SIZE),
                   l4IntCore, &loadGeneric, &storeGeneric);


  // L4_CORE_WAKEUP: power and reset manager
  device * prm = (device*)mallocBytes(sizeof(device));
  if (prm == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate Power/reset manager.\n");
  }
  else
  {
    memset((void*)prm, 0x0, sizeof(device));
  }
  initPrm();
  initialiseDevice(prm, "PRM", FALSE,
                   PRM, (u32int)(PRM -1 + PRM_SIZE),
                   l4CoreWakeupInt, &loadPrm, &storePrm);


  // L4_CORE_WAKEUP: general purpose I/O 1
  device * gpio1 = (device*)mallocBytes(sizeof(device));
  if (gpio1 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPIO1\n");
  }
  else
  {
    memset((void*)gpio1, 0x0, sizeof(device));
  }
  initGpio(1);
  initialiseDevice(gpio1, "GPIO1", FALSE,
                   GPIO1, (u32int)(GPIO1 -1 + GPIO1_SIZE),
                   l4CoreWakeupInt, &loadGpio, &storeGpio);


  // L4_CORE_WAKEUP: watchdog timer 2
  device * wdtimer2 = (device*)mallocBytes(sizeof(device));
  if (wdtimer2 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate WDTIMER2\n");
  }
  else
  {
    memset((void*)wdtimer2, 0x0, sizeof(device));
  }
  initialiseDevice(wdtimer2, "WDTIMER2", FALSE,
                   WDTIMER2, (u32int)(WDTIMER2 -1 + WDTIMER2_SIZE),
                   l4CoreWakeupInt, &loadGeneric, &storeGeneric);


  // L4_CORE_WAKEUP: general purpose timer 1
  device * gptimer1 = (device*)mallocBytes(sizeof(device));
  if (gptimer1 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPTIMER1\n");
  }
  else
  {
    memset((void*)gptimer1, 0x0, sizeof(device));
  }
  initGPTimer();
  initialiseDevice(gptimer1, "GPTIMER1", FALSE,
                   GPTIMER1, (u32int)(GPTIMER1 -1 + GPTIMER1_SIZE),
                   l4CoreWakeupInt, &loadGPTimer, &storeGPTimer);


  // L4_CORE_WAKEUP: 32 Kiloherz synchronised timer
  device * timer32k = (device*)mallocBytes(sizeof(device));
  if (timer32k == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate timer32k\n");
  }
  else
  {
    memset((void*)timer32k, 0x0, sizeof(device));
  }
  initTimer32k();
  initialiseDevice(timer32k, "32kTIMER", FALSE,
                   TIMER_32K, (u32int)(TIMER_32K -1 + TIMER_32K_SIZE),
                   l4CoreWakeupInt, &loadTimer32k, &storeTimer32k);


  // L4 interconnect: L4 interconnect peripherals
  device * l4IntPer = (device*)mallocBytes(sizeof(device));
  if (l4IntPer == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate L4 Interconnect peripheral bus\n");
  }
  else
  {
    memset((void*)l4IntPer, 0x0, sizeof(device));
  }
  initialiseDevice(l4IntPer, "L4_INT_PER", TRUE,
                   Q1_L4_INT_PER, (u32int)(Q1_L4_INT_PER -1 + Q1_L4_INT_PER_SIZE),
                   l4Interconnect, &loadGeneric, &storeGeneric);


  // L4INT_PER: uart3
  device * uart3 = (device*)mallocBytes(sizeof(device));
  if (uart3 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate UART3\n");
  }
  else
  {
    memset((void*)uart3, 0x0, sizeof(device));
  }
  initUart(3);
  initialiseDevice(uart3, "UART3", FALSE,
                   UART3, (u32int)(UART3 -1 + UART3_SIZE),
                   l4IntPer, &loadUart, &storeUart);


  // L4_INT_PER: general purpose I/O 2
  device * gpio2 = (device*)mallocBytes(sizeof(device));
  if (gpio2 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPIO2\n");
  }
  else
  {
    memset((void*)gpio2, 0x0, sizeof(device));
  }
  initGpio(2);
  initialiseDevice(gpio2, "GPIO2", FALSE,
                   GPIO2, (u32int)(GPIO2 -1 + GPIO2_SIZE),
                   l4IntPer, &loadGpio, &storeGpio);


  // L4_INT_PER: general purpose I/O 3
  device * gpio3 = (device*)mallocBytes(sizeof(device));
  if (gpio3 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPIO3\n");
  }
  else
  {
    memset((void*)gpio3, 0x0, sizeof(device));
  }
  initGpio(3);
  initialiseDevice(gpio3, "GPIO3", FALSE,
                   GPIO3, (u32int)(GPIO3 -1 + GPIO3_SIZE),
                   l4IntPer, &loadGpio, &storeGpio);


  // L4_INT_PER: general purpose I/O 4
  device * gpio4 = (device*)mallocBytes(sizeof(device));
  if (gpio4 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPIO4\n");
  }
  else
  {
    memset((void*)gpio4, 0x0, sizeof(device));
  }
  initGpio(4);
  initialiseDevice(gpio4, "GPIO4", FALSE,
                   GPIO4, (u32int)(GPIO4 -1 + GPIO4_SIZE),
                   l4IntPer, &loadGpio, &storeGpio);


  // L4_INT_PER: general purpose I/O 5
  device * gpio5 = (device*)mallocBytes(sizeof(device));
  if (gpio5 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPIO5\n");
  }
  else
  {
    memset((void*)gpio5, 0x0, sizeof(device));
  }
  initGpio(5);
  initialiseDevice(gpio5, "GPIO5", FALSE,
                   GPIO5, (u32int)(GPIO5 -1 + GPIO5_SIZE),
                   l4IntPer, &loadGpio, &storeGpio);


  // L4_INT_PER: general purpose I/O 6
  device * gpio6 = (device*)mallocBytes(sizeof(device));
  if (gpio6 == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate GPIO6\n");
  }
  else
  {
    memset((void*)gpio6, 0x0, sizeof(device));
  }
  initGpio(6);
  initialiseDevice(gpio6, "GPIO6", FALSE,
                   GPIO6, (u32int)(GPIO6 -1 + GPIO6_SIZE),
                   l4IntPer, &loadGpio, &storeGpio);

  // QUARTER 2
  device * q2bus = (device*)mallocBytes(sizeof(device));
  if (q2bus == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate Q2 bus\n");
  }
  else
  {
    memset((void*)q2bus, 0x0, sizeof(device));
  }
  initialiseDevice(q2bus, "Q2Bus", TRUE, QUARTER2, (u32int)(QUARTER2-1+QUARTER_SIZE),
                   topLevelBus, &loadGeneric, &storeGeneric);


  // Q2: sdram
  device * sdramModule = (device*)mallocBytes(sizeof(device));
  if (sdramModule == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate SDRAM module\n");
  }
  else
  {
    memset((void*)sdramModule, 0x0, sizeof(device));
  }
  initSdram();
  initialiseDevice(sdramModule, "SDRAM", FALSE,
                   Q2_SDRC_SMS, (u32int)(Q2_SDRC_SMS+Q2_SDRC_SMS_SIZE-1),
                   q2bus, &loadSdram, &storeSdram);


  // QUARTER 3
  device * q3bus = (device*)mallocBytes(sizeof(device));
  if (q3bus == 0)
  {
    DIE_NOW(0, "initialiseHardwareLibrary(): Failed to allocate Q3 bus\n");
  }
  else
  {
    memset((void*)q3bus, 0x0, sizeof(device));
  }
  initialiseDevice(q3bus, "Q3Bus", TRUE, QUARTER3, (u32int)(QUARTER3-1+QUARTER_SIZE),
                   topLevelBus, &loadGeneric, &storeGeneric);

  
  return topLevelBus;
}

void initialiseDevice(device * dev, char * devName, bool isBus,
                      u32int addrStart, u32int addrEnd,
                      device * parent, LOAD_FUNCTION ldFn, STORE_FUNCTION stFn)
{
#ifdef HARDWARE_LIB_DBG
  printf("Initialising device: ");
  printf(devName);
  printf("\n");
#endif

  int index = 0;
  dev->deviceName = devName;
  dev->isBus = isBus;
  dev->startAddressMapped = addrStart;
  dev->endAddressMapped = addrEnd;
  dev->parentDevice = 0;
  dev->nrOfAttachedDevs = 0;
  for (index = 0; index < MAX_NR_ATTACHED; index++)
  {
    dev->attachedDevices[index] = 0;
  }
  if (parent != 0)
  {
    // this is not the 'root' device, must be attached to something
    if (!attachDevice(parent, dev))
    {
      printf("Failed to attach device ");
      printf(devName);
      printf(" to device ");
      printf(parent->deviceName);
      DIE_NOW(0, "ERROR.");
    }
  }
  dev->loadFunction = ldFn;
  dev->storeFunction = stFn;
}

/**************************************
 * misc device functions              *
 **************************************/
bool attachDevice(device * parent, device * child)
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
#ifdef HARDWARE_LIB_DBG
    printf("Attached ");
    printf(child->deviceName);
    printf(" to ");
    printf(parent->deviceName);
    printf("\n");
#endif
    return TRUE;
  }
  return FALSE;
}

inline bool isAddressInDevice(u32int address, device * dev)
{
  if ( (address >= dev->startAddressMapped) && (address <= dev->endAddressMapped) )
  {
    // hit!!
    return TRUE;
  }
  return FALSE;
}

/**************************************
 * generic LOAD/STORE functions       *
 **************************************/
void storeGeneric(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  GCONTXT * gc = getGuestContext();
  u32int phyAddr = 0;
  if (gc->virtAddrEnabled)
  {
    phyAddr = getPhysicalAddress(gc->PT_shadow, address);
  } 
  else
  {
    phyAddr = getPhysicalAddress(gc->PT_physical, address);
  }

  if (dev->isBus)
  {
    // bus device
    int index = 0;
    for (index = 0; index < dev->nrOfAttachedDevs; index++)
    {
      if (isAddressInDevice(phyAddr, dev->attachedDevices[index]))
      {
        // hit the address range!
        dev->attachedDevices[index]->storeFunction(dev->attachedDevices[index], size, address, value);
        return;
      }
    }
    printf("Store to %s at address %08x physical %08x value %08x\n",
           dev->deviceName, address, phyAddr, value);
    DIE_NOW(gc, "No child of current device holds load address in range.");
  }
  else
  {
    // not a bus, end device
    printf("Store to %s at address %08x physical %08x value %08x\n",
           dev->deviceName, address, phyAddr, value);
    DIE_NOW(gc, "End device didn't implement custom store function!");
  }

}

u32int loadGeneric(device * dev, ACCESS_SIZE size, u32int address)
{
  GCONTXT * gc = getGuestContext();
  u32int phyAddr = 0;
  if (gc->virtAddrEnabled)
  {
    phyAddr = getPhysicalAddress(gc->PT_shadow, address);
  }
  else
  {
    phyAddr = getPhysicalAddress(gc->PT_physical, address);
  }

  if (dev->isBus)
  {
    // bus device
    int index = 0;
    for (index = 0; index < dev->nrOfAttachedDevs; index++)
    {
      if (isAddressInDevice(phyAddr, dev->attachedDevices[index]))
      {
        // hit the address range!
        return dev->attachedDevices[index]->loadFunction(dev->attachedDevices[index], size, address);
      }
    }
    printf("Load from %s address %08x physical %08x\n", dev->deviceName, address, phyAddr);
    DIE_NOW(gc, "No child of current device holds load address in range.");
  }
  else
  {
    // not a bus, end device
    printf("Load from %s address %08x physical %08x\n", dev->deviceName, address, phyAddr);
    DIE_NOW(gc, "End device didn't implement custom load function!");
  }

  return 0;
}
