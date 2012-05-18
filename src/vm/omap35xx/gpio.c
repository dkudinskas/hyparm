#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#ifdef CONFIG_GUEST_FREERTOS
# include "drivers/beagle/beGPIO.h"
#endif

#include "guestManager/guestContext.h"

#include "vm/omap35xx/gpio.h"


void initGpio(u32int gpioNumber)
{
  GCONTXT* context = getGuestContext();

  context->vm->gpio[gpioNumber - 1] = (struct Gpio *)calloc(1, sizeof(struct Gpio));
  if (!context->vm->gpio[gpioNumber - 1])
  {
    DIE_NOW(NULL, "Failed to allocate Gpio.");
  }

  DEBUG(VP_OMAP_35XX_GPIO, "initGpio: GPIO%x @ %p" EOL, 
        gpioNumber, context->vm->gpio[gpioNumber - 1]);

  resetGpio(gpioNumber);
}

void resetGpio(u32int num)
{
  GCONTXT* context = getGuestContext();
  struct Gpio* gpio = context->vm->gpio[num-1];
  
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
u32int loadGpio(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int regOffset = 0;
  u32int gpioNum = 0;
  switch (phyAddr & 0xFFFFF000)
  {
    case GPIO1:
    {
      gpioNum = 0;
      regOffset = phyAddr - GPIO1;
      break;
    }
    case GPIO2:
    {
      gpioNum = 1;
      regOffset = phyAddr - GPIO2;
      break;
    }
    case GPIO3:
    {
      gpioNum = 2;
      regOffset = phyAddr - GPIO3;
      break;
    }
    case GPIO4:
    {
      gpioNum = 3;
      regOffset = phyAddr - GPIO4;
      break;
    }
    case GPIO5:
    {
      gpioNum = 4;
      regOffset = phyAddr - GPIO5;
      break;
    }
    case GPIO6:
    {
      gpioNum = 5;
      regOffset = phyAddr - GPIO6;
      break;
    }
    default:
      DIE_NOW(NULL, "GPIO: loadGpio - invalid gpio number for base address");
  }

  GCONTXT* context = getGuestContext();
  struct Gpio* gpio = context->vm->gpio[gpioNum];
  u32int val = 0;
  switch (regOffset)
  {
    case GPIO_REVISION:
    {
      val = gpio->gpioRevision;
      break;
    }
    case GPIO_SYSCONFIG:
    {
      val = gpio->gpioSysConfig;
      break;
    }
    case GPIO_SYSSTATUS:
    {
      val = gpio->gpioSysStatus;
      break;
    }
    case GPIO_IRQSTATUS1:
    {
      val = gpio->gpioIrqStatus1;
      break;
    }
    case GPIO_IRQENABLE1:
    {
      val = gpio->gpioIrqEnable1;
      break;
    }
    case GPIO_IRQSTATUS2:
    {
      val = gpio->gpioIrqStatus2;
      break;
    }
    case GPIO_IRQENABLE2:
    {
      val = gpio->gpioIrqEnable2;
      break;
    }
    case GPIO_CTRL:
    {
      val = gpio->gpioCtrl;
      break;
    }
    case GPIO_WAKEUPENABLE:
    case GPIO_OE:
    case GPIO_DATAIN:
    case GPIO_DATAOUT:
#ifdef CONFIG_GUEST_FREERTOS
    {
      /* FreeRTOS GPIO status */
      val = gpio->gpioDataOut;
      if (gpioNum == 4 || gpioNum == 5)
      {
        val = beGetGPIO(regOffset, gpioNum);
      }
      break;
    }
#endif
    case GPIO_CLEARDATAOUT:
#ifdef CONFIG_GUEST_FREERTOS
    {
      val = gpio->gpioClearDataOut;
      if (gpioNum == 4 || gpioNum == 5)
      {
        val = beGetGPIO(regOffset, gpioNum);
      }
      break;
    }
#endif
    case GPIO_SETDATAOUT:
#ifdef CONFIG_GUEST_FREERTOS
    {
      val = gpio->gpioSetDataOut;
      if (gpioNum == 4 || gpioNum == 5)
      {
        val = beGetGPIO(regOffset, gpioNum);
      }
      break;
    }
#endif
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
    {
      printf("GPIO: load from unimplemented register %x" EOL, regOffset);
      DIE_NOW(NULL, "panic.");
    }
    default:
      DIE_NOW(NULL, "Gpio: load on invalid register.");
  }
  DEBUG(VP_OMAP_35XX_GPIO, "%s load from pAddr: %.8x, vAddr: %.8x access size %x val = %.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, val);
  return val;
}


/* top store function */
void storeGpio(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_GPIO, "%s store to pAddr: %.8x, vAddr: %.8x, access size: %x, val %.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  u32int regOffset = 0;
  u32int gpioNum = 0;
  switch (phyAddr & 0xFFFFF000)
  {
    case GPIO1:
    {
      gpioNum = 0;
      regOffset = phyAddr - GPIO1;
      break;
    }
    case GPIO2:
    {
      gpioNum = 1;
      regOffset = phyAddr - GPIO2;
      break;
    }
    case GPIO3:
    {
      gpioNum = 2;
      regOffset = phyAddr - GPIO3;
      break;
    }
    case GPIO4:
    {
      gpioNum = 3;
      regOffset = phyAddr - GPIO4;
      break;
    }
    case GPIO5:
    {
      gpioNum = 4;
      regOffset = phyAddr - GPIO5;
      break;
    }
    case GPIO6:
    {
      gpioNum = 5;
      regOffset = phyAddr - GPIO6;
      break;
    }
    default:
      DIE_NOW(NULL, "GPIO: storeGpio - invalid gpio number for base address");
  }

  GCONTXT* context = getGuestContext();
  struct Gpio* gpio = context->vm->gpio[gpioNum];
  switch (regOffset)
  {
    case GPIO_REVISION:
    {
      DIE_NOW(NULL, "GPIO: storing to a R/O reg (revision)");
      break;
    }
    case GPIO_SYSCONFIG:
    {
      if ((value & GPIO_SYSCONFIG_SOFTRESET) == GPIO_SYSCONFIG_SOFTRESET)
      {
        DEBUG(VP_OMAP_35XX_GPIO, "GPIO: soft reset" EOL);
        resetGpio(gpioNum);
      }
      else
      {
        gpio->gpioSysConfig = (value & ~GPIO_SYSCONFIG_RESERVED);
      }
      break;
    }
    case GPIO_SYSSTATUS:
    {
      DIE_NOW(NULL, "GPIO: storing to a R/O reg (sysStatus)");
      break;
    }
    case GPIO_IRQSTATUS1:
    {
      if (value != 0xffffffff)
      {
        DIE_NOW(NULL, "GPIO: clearing random interrupts. have a look!...");
      }
      gpio->gpioIrqStatus1 = gpio->gpioIrqStatus1 & ~value;
      break;
    }
    case GPIO_IRQENABLE1:
    {
      if (value != 0)
      {
        DIE_NOW(NULL, "GPIO: enabling interrupt! have a look at this...");
      }
      gpio->gpioIrqEnable1 = value;
      break;
    }
    case GPIO_IRQSTATUS2:
    {
      if (value != 0xffffffff)
      {
        DIE_NOW(NULL, "GPIO: clearing random interrupts. have a look!...");
      }
      gpio->gpioIrqStatus2 = gpio->gpioIrqStatus2 & ~value;
      break;
    }
    case GPIO_IRQENABLE2:
    {
      if (value != 0)
      {
        DIE_NOW(NULL, "GPIO: enabling interrupt! have a look at this...");
      }
      gpio->gpioIrqEnable2 = value;
      break;
    }
    case GPIO_CTRL:
    {
      if ((value & GPIO_CTRL_DISABLEMOD) == GPIO_CTRL_DISABLEMOD)
      {
        DIE_NOW(NULL, "GPIO: disabling module! investigate.");
      }
      gpio->gpioCtrl = value & ~GPIO_CTRL_RESERVED;
      break;
    }
    case GPIO_OE:
#ifdef CONFIG_GUEST_FREERTOS
    {
      /* value can be any 32-bit number */
      gpio->gpioOE = value;
      /* FreeRTOS initialization */
      if (gpioNum == 4 || gpioNum == 5)
      {
        beStoreGPIO(regOffset, value, gpioNum);
      }
      break;
    }
#endif
    case GPIO_CLEARDATAOUT:
#ifdef CONFIG_GUEST_FREERTOS
    {
      /* value can be any 32-bit number */
      gpio->gpioClearDataOut = value;
      gpio->gpioDataOut = ~(value);
      /* FreeRTOS initialization */
      if (gpioNum == 4 || gpioNum == 5)
      {
        beStoreGPIO(regOffset, value, gpioNum);
      }
      break;
    }
#endif
    case GPIO_SETDATAOUT:
#ifdef CONFIG_GUEST_FREERTOS
    {
      /* value can be any 32-bit number */
      gpio->gpioSetDataOut = value;
      gpio->gpioDataOut = value;
      /* FreeRTOS initialization */
      if (gpioNum == 4 || gpioNum == 5)
      {
        beStoreGPIO(regOffset, value, gpioNum);
      }
      break;
    }
#endif
    case GPIO_WAKEUPENABLE:
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
    {
      printf("GPIO: store to unimplemented register %x" EOL, regOffset);
      DIE_NOW(NULL, "panic.");
    }
    default:
      DIE_NOW(NULL, "Gpio: store to invalid register.");
  }
}

