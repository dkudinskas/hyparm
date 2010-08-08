#include "hardwareLibrary.h"
#include "sdram.h"
#include "sramInternal.h"
#include "timer32k.h"
#include "prm.h"
#include "clockManager.h"
#include "sysControlModule.h"
#include "guestContext.h"
#include "gpmc.h"
#include "intc.h"

extern GCONTXT * getGuestContext(void);

// top level device..
device topLevelBus;
char * topLevelBusName = "TopLevelBus";

/********************************************************/
/*      lets split all address space into 4 quarters    */
/********************************************************/
// QUARTER 0
device q0bus;
char * q0busName = "Q0Bus";
/********************************************************/
// QUARTER 1
device q1bus;
char * q1busName = "Q1Bus";
// Q1: ON CHIP MEMORY
device onChipMemory;
char * onChipMemoryName = "ON_CHIP_MEMORY";
// OCM: secure boot rom
device bootRomSecure;
char * bootRomSecureName = "BOOT_ROM_SECURE";
// OCM: public boot rom
device bootRomPublic;
char * bootRomPublicName = "BOOT_ROM_PUBLIC";
// OCM: internal sram
device sramInternal;
char * sramInternalName = "SRAM_INTERNAL";
// LEVEL4 INTERCONNECT
device l4Interconnect;
char * l4InterconnectName = "L4_INTERCONNECT";
// L4INT: core
device l4IntCore;
char * l4IntCoreName = "L4_INTERCONNECT_CORE";
// L4INT_CORE: system control module
device sysCtrlMod;
char * sysCtrlModName = "SYSCTRL_MOD";
// L4INT_CORE: clock manager (and DPLL)
device clockManager;
char * clockManagerName = "CLK_MAN";
// L4INT_CORE: interrupt controller
device intc;
char * intcName = "INTC";
// L4INT_CORE: core wakeup interconnect
device l4CoreWakeupInt;
char * l4CoreWakeupIntName = "L4_CORE_WAKEUP_INT";

// LEVEL3 INTERCONNECT
device l3Interconnect;
char * l3InterconnectName = "L3_INTERCONNECT";
// L3INT: general purpose memory controller
device gpmcModule;
char * gpmcModuleName = "GPMC";

// L4_CORE_WAKEUP: power and reset manager
device prm;
char * prmName = "PMR";
// L4_CORE_WAKEUP: general purpose I/O 1
device gpio1;
char * gpio1Name = "GPIO1";
// L4_CORE_WAKEUP: watchdog timer 2
device wdtimer2;
char * wdtimer2Name = "WDTIMER2";
// L4_CORE_WAKEUP: general purpose timer 1
device gptimer1;
char * gptimer1Name = "GPTIMER1";
// L4_CORE_WAKEUP: 32 Kiloherz synchronised timer
device timer32k;
char * timer32kName = "32kTIMER";
/********************************************************/
// QUARTER 2
device q2bus;
char * q2busName = "Q2Bus";
// Q2: sdram
device sdram;
char * sdramName = "SDRAM";
/********************************************************/
// QUARTER 3
device q3bus;
char * q3busName = "Q3Bus";


device * initialiseHardwareLibrary()
{
#ifdef HARDWARE_LIB_DBG
  serial_putstring("Initialising device library...");
  serial_newline();
#endif

  /********************************************************/
  /********************************************************/
  // top level device, everything hangs on it
  initialiseDevice(&topLevelBus, topLevelBusName, TRUE, 0, 0xFFFFFFFF,
                      0, &loadGeneric, &storeGeneric);
  /********************************************************/
  /********************************************************/
  // QUARTER 0
  initialiseDevice(&q0bus, q0busName, TRUE, QUARTER0, (u32int)(QUARTER0+QUARTER_SIZE-1),
                      &topLevelBus, &loadGeneric, &storeGeneric);
  // nothing lives on Q0 yet...
  /********************************************************/
  // QUARTER 1
  initialiseDevice(&q1bus, q1busName, TRUE, QUARTER1, (u32int)(QUARTER1-1+QUARTER_SIZE),
                      &topLevelBus, &loadGeneric, &storeGeneric);
  // Q1: ON CHIP MEMORY (OCM, parent Q1)
  initialiseDevice(&onChipMemory, onChipMemoryName, TRUE,
                   Q1_ON_CHIP_MEMORY, (u32int)(Q1_ON_CHIP_MEMORY+Q1_ON_CHIP_MEMORY_SIZE-1),
                   &q1bus, &loadGeneric, &storeGeneric);
  // OCM: secure boot rom (parent ocm)
  initialiseDevice(&bootRomSecure, bootRomSecureName, FALSE,
              Q1_OCM_BOOT_ROM_SECURE, (u32int)(Q1_OCM_BOOT_ROM_SECURE+Q1_OCM_BOOT_ROM_SECURE_SIZE-1),
              &onChipMemory, &loadGeneric, &storeGeneric);
  // OCM: public boot rom
  initialiseDevice(&bootRomPublic, bootRomPublicName, FALSE,
              Q1_OCM_BOOT_ROM_PUBLIC, (u32int)(Q1_OCM_BOOT_ROM_PUBLIC+Q1_OCM_BOOT_ROM_PUBLIC_SIZE-1),
              &onChipMemory, &loadGeneric, &storeGeneric);
  // OCM: internal sram
  initialiseDevice(&sramInternal, sramInternalName, FALSE,
              Q1_OCM_SRAM_INTERNAL, (u32int)(Q1_OCM_SRAM_INTERNAL+Q1_OCM_SRAM_INTERNAL_SIZE-1),
              &onChipMemory, &loadSramInternal, &storeSramInternal);

  // Q1: LEVEL3 INTERCONNECT (L3INT, parent Q1)
  initialiseDevice(&l3Interconnect, l3InterconnectName, TRUE,
                   Q1_L3_INTERCONNECT, (u32int)(Q1_L3_INTERCONNECT+Q1_L3_INTERCONNECT_SIZE-1),
                   &q1bus, &loadGeneric, &storeGeneric);
  // L3INT: general purpose memory controller
  initGpmc();
  initialiseDevice(&gpmcModule, gpmcModuleName, FALSE,
                   Q1_L3_GPMC, (u32int)(Q1_L3_GPMC-1+Q1_L3_GPMC_SIZE),
                   &l3Interconnect, &loadGpmc, &storeGpmc);

  // Q1: LEVEL4 INTERCONNECT (L4INT, parent Q1)
  initialiseDevice(&l4Interconnect, l4InterconnectName, TRUE,
                   Q1_L4_INTERCONNECT, (u32int)(Q1_L4_INTERCONNECT+Q1_L4_INTERCONNECT_SIZE-1),
                   &q1bus, &loadGeneric, &storeGeneric);
  // L4INT: core
  initialiseDevice(&l4IntCore, l4IntCoreName, TRUE,
                   Q1_L4_INT_CORE, (u32int)(Q1_L4_INT_CORE-1+Q1_L4_INT_CORE_SIZE),
                   &l4Interconnect, &loadGeneric, &storeGeneric);

  // L4INT_CORE: system control module
  initSysControlModule();
  initialiseDevice(&sysCtrlMod, sysCtrlModName, FALSE,
                   SYS_CONTROL_MODULE, (u32int)(SYS_CONTROL_MODULE -1 + SYS_CONTROL_MODULE_SIZE),
                   &l4IntCore, &loadSysCtrlModule, &storeSysCtrlModule);
  // L4INT_CORE: clock manager (and DPLL)
  initClockManager();
  initialiseDevice(&clockManager, clockManagerName, FALSE,
                   CLOCK_MANAGER, (u32int)(CLOCK_MANAGER -1 + CLOCK_MANAGER_SIZE),
                   &l4IntCore, &loadClockManager, &storeClockManager);
  // L4INT_CORE: interrupt controller
  initialiseDevice(&intc, intcName, FALSE,
                   INTERRUPT_CONTROLLER, (u32int)(INTERRUPT_CONTROLLER -1 + INTERRUPT_CONTROLLER_SIZE),
                   &l4IntCore, &loadIntc, &storeIntc);
  // L4INT_CORE: core wakeup interconnect
  initIntc();
  initialiseDevice(&l4CoreWakeupInt, l4CoreWakeupIntName, TRUE,
                   L4_CORE_WAKEUP_INT, (u32int)(L4_CORE_WAKEUP_INT-1+L4_CORE_WAKEUP_INT_SIZE),
                   &l4IntCore, &loadGeneric, &storeGeneric);
  // L4_CORE_WAKEUP: power and reset manager
  initPrm();
  initialiseDevice(&prm, prmName, FALSE,
                   PRM, (u32int)(PRM -1 + PRM_SIZE),
                   &l4CoreWakeupInt, &loadPrm, &storePrm);
  // L4_CORE_WAKEUP: general purpose I/O 1
  initialiseDevice(&gpio1, gpio1Name, FALSE,
                   GPIO1, (u32int)(GPIO1 -1 + GPIO1_SIZE),
                   &l4CoreWakeupInt, &loadGeneric, &storeGeneric);
  // L4_CORE_WAKEUP: watchdog timer 2
  initialiseDevice(&wdtimer2, wdtimer2Name, FALSE,
                   WDTIMER2, (u32int)(WDTIMER2 -1 + WDTIMER2_SIZE),
                   &l4CoreWakeupInt, &loadGeneric, &storeGeneric);
  // L4_CORE_WAKEUP: general purpose timer 1
  initialiseDevice(&gptimer1, gptimer1Name, FALSE,
                   GPTIMER1, (u32int)(GPTIMER1 -1 + GPTIMER1_SIZE),
                   &l4CoreWakeupInt, &loadGeneric, &storeGeneric);
  // L4_CORE_WAKEUP: 32 Kiloherz synchronised timer
  initTimer32k();
  initialiseDevice(&timer32k, timer32kName, FALSE,
                   TIMER_32K, (u32int)(TIMER_32K -1 + TIMER_32K_SIZE),
                   &l4CoreWakeupInt, &loadTimer32k, &storeTimer32k);
  /********************************************************/
  // QUARTER 2
  initialiseDevice(&q2bus, q2busName, TRUE, QUARTER2, (u32int)(QUARTER2-1+QUARTER_SIZE),
                      &topLevelBus, &loadGeneric, &storeGeneric);
  // Q2: sdram
  initialiseDevice(&sdram, sdramName, FALSE,
              Q2_SDRC_SMS, (u32int)(Q2_SDRC_SMS+Q2_SDRC_SMS_SIZE-1),
              &q2bus, &loadSdram, &storeSdram);
  /********************************************************/
  // QUARTER 3
  initialiseDevice(&q3bus, q3busName, TRUE, QUARTER3, (u32int)(QUARTER3-1+QUARTER_SIZE),
                      &topLevelBus, &loadGeneric, &storeGeneric);
  // no devices so far on quarter 3
  /********************************************************/
  
  return &topLevelBus;
}

void initialiseDevice(device * dev, char * devName, bool isBus,
                      u32int addrStart, u32int addrEnd,
                      device * parent, LOAD_FUNCTION ldFn, STORE_FUNCTION stFn)
{
#ifdef HARDWARE_LIB_DBG
  serial_putstring("Initialising device: ");
  serial_putstring(devName);
  serial_newline();
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
      serial_putstring("Failed to attach device ");
      serial_putstring(devName);
      serial_putstring(" to device ");
      serial_putstring(parent->deviceName);
      serial_ERROR("ERROR.");
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
    serial_putstring("Attached ");
    serial_putstring(child->deviceName);
    serial_putstring(" to ");
    serial_putstring(parent->deviceName);
    serial_newline();
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
    serial_putstring("Store to: ");
    serial_putstring(dev->deviceName);
    serial_putstring(" at address ");
    serial_putint(address);
    serial_putstring(" physical ");
    serial_putint(phyAddr);
    serial_putstring(" value ");
    serial_putint(value);
    serial_newline();
    serial_ERROR("No child of current device holds load address in range.");
  }
  else
  {
    // not a bus, end device
    serial_putstring("Store to: ");
    serial_putstring(dev->deviceName);
    serial_putstring(" at address ");
    serial_putint(address);
    serial_putstring(" physical ");
    serial_putint(phyAddr);
    serial_putstring(" value ");
    serial_putint(value);
    serial_newline();
    serial_ERROR("End device didn't implement custom store function!");
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
    serial_putstring("Load from: ");
    serial_putstring(dev->deviceName);
    serial_putstring(" from address ");
    serial_putint(address);
    serial_putstring(" physical ");
    serial_putint(phyAddr);
    serial_newline();
    dumpGuestContext(gc);
    serial_ERROR("No child of current device holds load address in range.");
  }
  else
  {
    // not a bus, end device
    serial_putstring("Load from: ");
    serial_putstring(dev->deviceName);
    serial_putstring(" from address ");
    serial_putint(address);
    serial_putstring(" physical ");
    serial_putint(phyAddr);
    serial_newline();
    serial_ERROR("End device didn't implement custom load function!");
  }

  return 0;
}
