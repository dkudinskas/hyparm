#include "gpio.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"

extern GCONTXT * getGuestContext(void);

struct Gpio * gpio;

void initGpio()
{
  gpio = (struct Gpio*)mallocBytes(sizeof(struct Gpio));
  if (gpio == 0)
  {
    serial_ERROR("Failed to allocate Gpio.");
  }
  else
  {
    memset((void*)gpio, 0x0, sizeof(struct Gpio));
#ifdef GPIO_DBG
    serial_putstring("Initializing GPIO at 0x");
    serial_putint((u32int)gpio);
    serial_newline();
#endif
  }
  
  resetGpio();
}

void resetGpio()
{
  // reset registers to default values
  gpio->gpioRevision        = 0x00000025;
  gpio->gpioSysConfig       = 0x00000000;
  gpio->gpioSysStatus       = 0x00000001;
  gpio->gpioIrqStatus1      = 0x00000000;
  gpio->gpioIrqEnable1      = 0x00000000;
  gpio->gpioWakeupEnable    = 0x00000000;
  gpio->gpioIrqStatus2      = 0x00000000;
  gpio->gpioIrqEnable2      = 0x00000000;
  gpio->gpioCtrl            = 0x00000002;
  gpio->gpioOE              = 0x00000000;
  gpio->gpioDataIn          = 0x00000000;
  gpio->gpioDataOut         = 0x00000000;
  gpio->gpioLvlDetect0      = 0x00000000;
  gpio->gpioLvlDetect1      = 0x00000000;
  gpio->gpioRisingDetect    = 0x00000000;
  gpio->gpioFallingDetect   = 0x00000000;
  gpio->gpioDebounceEnable  = 0x00000000;
  gpio->gpioDebouncingTime  = 0x00000000;
  gpio->gpioClearIrqEnable1 = 0x00000000;
  gpio->gpioSetIrqEnable1   = 0x00000000;
  gpio->gpioClearIrqEnable2 = 0x00000000;
  gpio->gpioSetIrqEnable2   = 0x00000000;
  gpio->gpioClearWkuEnable  = 0x00000000;
  gpio->gpioSetWkuEnable    = 0x00000000;
  gpio->gpioClearDataOut    = 0x00000000;
  gpio->gpioSetDataOut      = 0x00000000;
  
  // set reset complete bit
  gpio->gpioSysStatus = GPIO_SYSSTATUS_RESETDONE;
}

/* load function */
u32int loadGpio(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  if (size != WORD)
  {
    // only word access allowed in these modules
    serial_ERROR("Gpio: invalid access size.");
  }

  u32int regOffset = phyAddr - GPIO1;
  u32int val = 0;
  switch (regOffset)
  {
    case GPIO_REVISION:
      val = gpio->gpioRevision;
      break;
    case GPIO_SYSCONFIG:
      val = gpio->gpioSysConfig;
      break;
    case GPIO_SYSSTATUS:
      val = gpio->gpioSysStatus;
      break;
    case GPIO_IRQSTATUS1:
      val = gpio->gpioIrqStatus1;
      break;
    case GPIO_IRQENABLE1:
      val = gpio->gpioIrqEnable1;
      break;
    case GPIO_IRQSTATUS2:
      val = gpio->gpioIrqStatus2;
      break;
    case GPIO_IRQENABLE2:
      val = gpio->gpioIrqEnable2;
      break;
    case GPIO_CTRL:
      val = gpio->gpioCtrl;
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

  u32int regOffset = phyAddr - GPIO1;
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
        resetGpio();
      }
      else
      {
        gpio->gpioSysConfig = (value & ~GPIO_SYSCONFIG_RESERVED);
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
      gpio->gpioIrqStatus1 = gpio->gpioIrqStatus1 & ~value;
      break;
    case GPIO_IRQENABLE1:
      if (value != 0)
      {
        serial_ERROR("GPIO: enabling interrupt! have a look at this...");
      }
      gpio->gpioIrqEnable1 = value;
      break;
    case GPIO_IRQSTATUS2:
      if (value != 0xffffffff)
      {
        serial_ERROR("GPIO: clearing random interrupts. have a look!...");
      }
      gpio->gpioIrqStatus2 = gpio->gpioIrqStatus2 & ~value;
      break;
    case GPIO_IRQENABLE2:
      if (value != 0)
      {
        serial_ERROR("GPIO: enabling interrupt! have a look at this...");
      }
      gpio->gpioIrqEnable2 = value;
      break;
    case GPIO_CTRL:
      if ((value & GPIO_CTRL_DISABLEMOD) == GPIO_CTRL_DISABLEMOD)
      {
        serial_ERROR("GPIO: disabling module! investigate.");
      }
      gpio->gpioCtrl = value & ~GPIO_CTRL_RESERVED;
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

