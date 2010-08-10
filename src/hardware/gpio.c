#include "gpio.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"

extern GCONTXT * getGuestContext(void);

struct Gpio * gpio[6];

void initGpio(u32int gpioNumber)
{
  gpio[gpioNumber] = (struct Gpio*)mallocBytes(sizeof(struct Gpio));
  if (gpio[gpioNumber] == 0)
  {
    serial_ERROR("Failed to allocate Gpio.");
  }
  else
  {
    memset((void*)gpio[gpioNumber], 0x0, sizeof(struct Gpio));
#ifdef GPIO_DBG
    serial_putstring("Initializing GPIO");
    serial_putint_nozeros(gpioNumber);
    serial_putstring(" at 0x");
    serial_putint((u32int)gpio[gpioNumber]);
    serial_newline();
#endif
  }
  
  resetGpio(gpioNumber);
}

void resetGpio(u32int num)
{
  // reset registers to default values
  gpio[num]->gpioRevision        = 0x00000025;
  gpio[num]->gpioSysConfig       = 0x00000000;
  gpio[num]->gpioSysStatus       = 0x00000001;
  gpio[num]->gpioIrqStatus1      = 0x00000000;
  gpio[num]->gpioIrqEnable1      = 0x00000000;
  gpio[num]->gpioWakeupEnable    = 0x00000000;
  gpio[num]->gpioIrqStatus2      = 0x00000000;
  gpio[num]->gpioIrqEnable2      = 0x00000000;
  gpio[num]->gpioCtrl            = 0x00000002;
  gpio[num]->gpioOE              = 0x00000000;
  gpio[num]->gpioDataIn          = 0x00000000;
  gpio[num]->gpioDataOut         = 0x00000000;
  gpio[num]->gpioLvlDetect0      = 0x00000000;
  gpio[num]->gpioLvlDetect1      = 0x00000000;
  gpio[num]->gpioRisingDetect    = 0x00000000;
  gpio[num]->gpioFallingDetect   = 0x00000000;
  gpio[num]->gpioDebounceEnable  = 0x00000000;
  gpio[num]->gpioDebouncingTime  = 0x00000000;
  gpio[num]->gpioClearIrqEnable1 = 0x00000000;
  gpio[num]->gpioSetIrqEnable1   = 0x00000000;
  gpio[num]->gpioClearIrqEnable2 = 0x00000000;
  gpio[num]->gpioSetIrqEnable2   = 0x00000000;
  gpio[num]->gpioClearWkuEnable  = 0x00000000;
  gpio[num]->gpioSetWkuEnable    = 0x00000000;
  gpio[num]->gpioClearDataOut    = 0x00000000;
  gpio[num]->gpioSetDataOut      = 0x00000000;
  
  // set reset complete bit
  gpio[num]->gpioSysStatus = GPIO_SYSSTATUS_RESETDONE;
}


/* load function */
u32int loadGpio(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  u32int regOffset = 0;
  u32int gpioNum = 0;
  switch (phyAddr & 0xFFFFF000)
  {
    case GPIO1:
      gpioNum = 1;
      regOffset = phyAddr - GPIO1;
      break;
    case GPIO2:
      gpioNum = 2;
      regOffset = phyAddr - GPIO2;
      break;
    case GPIO3:
      gpioNum = 3;
      regOffset = phyAddr - GPIO3;
      break;
    case GPIO4:
      gpioNum = 4;
      regOffset = phyAddr - GPIO4;
      break;
    case GPIO5:
      gpioNum = 5;
      regOffset = phyAddr - GPIO5;
      break;
    case GPIO6:
      gpioNum = 6;
      regOffset = phyAddr - GPIO6;
      break;
    default:
      serial_ERROR("GPIO: loadGpio - invalid gpio number for base address");
  }

  u32int val = 0;
  switch (regOffset)
  {
    case GPIO_REVISION:
      val = gpio[gpioNum]->gpioRevision;
      break;
    case GPIO_SYSCONFIG:
      val = gpio[gpioNum]->gpioSysConfig;
      break;
    case GPIO_SYSSTATUS:
      val = gpio[gpioNum]->gpioSysStatus;
      break;
    case GPIO_IRQSTATUS1:
      val = gpio[gpioNum]->gpioIrqStatus1;
      break;
    case GPIO_IRQENABLE1:
      val = gpio[gpioNum]->gpioIrqEnable1;
      break;
    case GPIO_IRQSTATUS2:
      val = gpio[gpioNum]->gpioIrqStatus2;
      break;
    case GPIO_IRQENABLE2:
      val = gpio[gpioNum]->gpioIrqEnable2;
      break;
    case GPIO_CTRL:
      val = gpio[gpioNum]->gpioCtrl;
      break;
    case GPIO_WAKEUPENABLE:
    case GPIO_OE:
    case GPIO_DATAIN:
    case GPIO_DATAOUT:
    case GPIO_LEVELDETECT0:
    case GPIO_LEVELDETECT1:
    case GPIO_RISINGDETECT:
    case GPIO_FALLINGDETECT:
    case GPIO_DEBOUNCENABLE:
    case GPIO_DEBOUNCINGTIME:
    case GPIO_CLEARIRQENABLE1:
    case GPIO_SETIRQENABLE1:
    case GPIO_CLEARIRQENABLE2:
    case GPIO_SETIRQENABLE2:
    case GPIO_CLEARWKUENA:
    case GPIO_SETWKUENA:
    case GPIO_CLEARDATAOUT:
    case GPIO_SETDATAOUT:
      serial_putstring("GPIO: load from unimplemented register ");
      serial_putint_nozeros(regOffset);
      serial_ERROR(", panic.");
      break;
    default:
      serial_ERROR("Gpio: load on invalid register.");
  }
#ifdef GPIO_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_putstring(" val = ");
  serial_putint(val);
  serial_newline();
#endif
  return val;
}


/* top store function */
void storeGpio(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef GPIO_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" store to pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" aSize ");
  serial_putint((u32int)size);
  serial_putstring(" val ");
  serial_putint(value);
  serial_newline();
#endif

  u32int regOffset = 0;
  u32int gpioNum = 0;
  switch (phyAddr & 0xFFFFF000)
  {
    case GPIO1:
      gpioNum = 1;
      regOffset = phyAddr - GPIO1;
      break;
    case GPIO2:
      gpioNum = 2;
      regOffset = phyAddr - GPIO2;
      break;
    case GPIO3:
      gpioNum = 3;
      regOffset = phyAddr - GPIO3;
      break;
    case GPIO4:
      gpioNum = 4;
      regOffset = phyAddr - GPIO4;
      break;
    case GPIO5:
      gpioNum = 5;
      regOffset = phyAddr - GPIO5;
      break;
    case GPIO6:
      gpioNum = 6;
      regOffset = phyAddr - GPIO6;
      break;
    default:
      serial_ERROR("GPIO: storeGpio - invalid gpio number for base address");
  }

  switch (regOffset)
  {
    case GPIO_REVISION:
      serial_ERROR("GPIO: storing to a R/O reg (revision)");
      break;
    case GPIO_SYSCONFIG:
      if ((value & GPIO_SYSCONFIG_SOFTRESET) == GPIO_SYSCONFIG_SOFTRESET)
      {
#ifdef GPIO_DBG
        serial_putstring("GPIO: soft reset.");
        serial_newline();
#endif
        resetGpio(gpioNum);
      }
      else
      {
        gpio[gpioNum]->gpioSysConfig = (value & ~GPIO_SYSCONFIG_RESERVED);
      }
      break;
    case GPIO_SYSSTATUS:
      serial_ERROR("GPIO: storing to a R/O reg (sysStatus)");
      break;
    case GPIO_IRQSTATUS1:
      if (value != 0xffffffff)
      {
        serial_ERROR("GPIO: clearing random interrupts. have a look!...");
      }
      gpio[gpioNum]->gpioIrqStatus1 = gpio[gpioNum]->gpioIrqStatus1 & ~value;
      break;
    case GPIO_IRQENABLE1:
      if (value != 0)
      {
        serial_ERROR("GPIO: enabling interrupt! have a look at this...");
      }
      gpio[gpioNum]->gpioIrqEnable1 = value;
      break;
    case GPIO_IRQSTATUS2:
      if (value != 0xffffffff)
      {
        serial_ERROR("GPIO: clearing random interrupts. have a look!...");
      }
      gpio[gpioNum]->gpioIrqStatus2 = gpio[gpioNum]->gpioIrqStatus2 & ~value;
      break;
    case GPIO_IRQENABLE2:
      if (value != 0)
      {
        serial_ERROR("GPIO: enabling interrupt! have a look at this...");
      }
      gpio[gpioNum]->gpioIrqEnable2 = value;
      break;
    case GPIO_CTRL:
      if ((value & GPIO_CTRL_DISABLEMOD) == GPIO_CTRL_DISABLEMOD)
      {
        serial_ERROR("GPIO: disabling module! investigate.");
      }
      gpio[gpioNum]->gpioCtrl = value & ~GPIO_CTRL_RESERVED;
      break;
    case GPIO_WAKEUPENABLE:
    case GPIO_OE:
    case GPIO_DATAIN:
    case GPIO_DATAOUT:
    case GPIO_LEVELDETECT0:
    case GPIO_LEVELDETECT1:
    case GPIO_RISINGDETECT:
    case GPIO_FALLINGDETECT:
    case GPIO_DEBOUNCENABLE:
    case GPIO_DEBOUNCINGTIME:
    case GPIO_CLEARIRQENABLE1:
    case GPIO_SETIRQENABLE1:
    case GPIO_CLEARIRQENABLE2:
    case GPIO_SETIRQENABLE2:
    case GPIO_CLEARWKUENA:
    case GPIO_SETWKUENA:
    case GPIO_CLEARDATAOUT:
    case GPIO_SETDATAOUT:
      serial_putstring("GPIO: store to unimplemented register ");
      serial_putint_nozeros(regOffset);
      serial_ERROR(", panic.");
      break;
    default:
      serial_ERROR("Gpio: store to invalid register.");
  }
}

