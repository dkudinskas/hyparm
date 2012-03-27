#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "drivers/beagle/beGPIO.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/gpio.h"


#define ADDRESS_MASK  0xFFFFF000


static inline s32int getIndexByAddress(u32int physicalAddress);
static void reset(u32int index);


struct Gpio *gpio[6];


void connectGpio(u32int gpioNumber, u32int physicalGpioNumber)
{
  gpio[gpioNumber - 1]->physicalId = physicalGpioNumber - 1;
}

void initGpio(u32int gpioNumber)
{
  u32int index = gpioNumber - 1;
  gpio[index] = (struct Gpio *)malloc(sizeof(struct Gpio));
  if (!gpio[index])
  {
    DIE_NOW(NULL, "Failed to allocate Gpio.");
  }

  memset(gpio[index], 0, sizeof(struct Gpio));
  gpio[index]->physicalId = -1;
  DEBUG(VP_OMAP_35XX_GPIO, "initGpio: GPIO%x @ %p" EOL, gpioNumber, gpio[index]);

  reset(index);
}

/*
 * Look up GPIO index by physical address.
 */
static inline s32int getIndexByAddress(u32int physicalAddress)
{
  switch (physicalAddress & ADDRESS_MASK)
  {
    case GPIO1:
      return 0;
    case GPIO2:
      return 1;
    case GPIO3:
      return 2;
    case GPIO4:
      return 3;
    case GPIO5:
      return 4;
    case GPIO6:
      return 5;
    default:
      return -1;
  }
}

/* load function */
u32int loadGpio(device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress)
{
  s32int index = getIndexByAddress(physicalAddress);
  u32int regOffset = physicalAddress & ~ADDRESS_MASK;
  if (index < 0)
  {
    DIE_NOW(NULL, "cannot translate physical address to GPIO number");
  }

  u32int val = 0;
  if (gpio[index]->physicalId < 0)
  {
    /*
     * This is a virtual GPIO without physical counterpart.
     */
    switch (regOffset)
    {
      case GPIO_REVISION:
        val = gpio[index]->gpioRevision;
        break;
      case GPIO_SYSCONFIG:
        val = gpio[index]->gpioSysConfig;
        break;
      case GPIO_SYSSTATUS:
        val = gpio[index]->gpioSysStatus;
        break;
      case GPIO_IRQSTATUS1:
        val = gpio[index]->gpioIrqStatus1;
        break;
      case GPIO_IRQENABLE1:
      case GPIO_CLEARIRQENABLE1:
      case GPIO_SETIRQENABLE1:
        val = gpio[index]->gpioIrqEnable1;
        break;
      case GPIO_WAKEUPENABLE:
      case GPIO_CLEARWKUENA:
      case GPIO_SETWKUENA:
        val = gpio[index]->gpioWakeupEnable;
        break;
      case GPIO_IRQSTATUS2:
        val = gpio[index]->gpioIrqStatus2;
        break;
      case GPIO_IRQENABLE2:
      case GPIO_CLEARIRQENABLE2:
      case GPIO_SETIRQENABLE2:
        val = gpio[index]->gpioIrqEnable2;
        break;
      case GPIO_CTRL:
        val = gpio[index]->gpioCtrl;
        break;
      case GPIO_OE:
        val = gpio[index]->gpioOE;
        break;
      case GPIO_DATAIN:
        printf("Warning: guest reading from disconnected GPIO" EOL);
        val = gpio[index]->gpioDataIn;
        break;
      case GPIO_DATAOUT:
      case GPIO_CLEARDATAOUT:
      case GPIO_SETDATAOUT:
        val = gpio[index]->gpioDataOut;
        break;
      case GPIO_LEVELDETECT0:
        val = gpio[index]->gpioLvlDetect0;
        break;
      case GPIO_LEVELDETECT1:
        val = gpio[index]->gpioLvlDetect1;
        break;
      case GPIO_RISINGDETECT:
        val = gpio[index]->gpioRisingDetect;
        break;
      case GPIO_FALLINGDETECT:
        val = gpio[index]->gpioFallingDetect;
        break;
      case GPIO_DEBOUNCENABLE:
        val = gpio[index]->gpioDebounceEnable;
        break;
      case GPIO_DEBOUNCINGTIME:
        val = gpio[index]->gpioDebouncingTime;
        break;
      default:
        DIE_NOW(NULL, "load on invalid register of disconnected GPIO");
    }
  }
  else
  {
    /*
     * This GPIO is mapped to a physical GPIO.
     */
    switch (regOffset)
    {
      case GPIO_REVISION:
      case GPIO_SYSCONFIG:
      case GPIO_SYSSTATUS:
      case GPIO_IRQSTATUS1:
      case GPIO_IRQENABLE1:
      case GPIO_WAKEUPENABLE:
      case GPIO_IRQSTATUS2:
      case GPIO_IRQENABLE2:
      case GPIO_CTRL:
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
        val = beGetGPIO(regOffset, gpio[index]->physicalId);
        break;
      default:
        DIE_NOW(NULL, "load on invalid register of connected GPIO");
    }
  }
  DEBUG(VP_OMAP_35XX_GPIO, "%s load from pAddr: %.8x, vAddr: %.8x access size %x val = %.8x" EOL,
      dev->deviceName, physicalAddress, virtualAddress, (u32int)size, val);
  return val;
}

static void reset(u32int index)
{
  /*
   * Reset registers to default values
   * WARNING: DO NOT CLEAR mapping to physical GPIO
   */
  gpio[index]->gpioRevision        = 0x00000025;
  gpio[index]->gpioSysConfig       = 0x00000000;
  gpio[index]->gpioSysStatus       = 0x00000001;
  gpio[index]->gpioIrqStatus1      = 0x00000000;
  gpio[index]->gpioIrqEnable1      = 0x00000000;
  gpio[index]->gpioWakeupEnable    = 0x00000000;
  gpio[index]->gpioIrqStatus2      = 0x00000000;
  gpio[index]->gpioIrqEnable2      = 0x00000000;
  gpio[index]->gpioCtrl            = 0x00000002;
  gpio[index]->gpioOE              = 0xFFFFFFFF;
  gpio[index]->gpioDataIn          = 0x00000000;
  gpio[index]->gpioDataOut         = 0x00000000;
  gpio[index]->gpioLvlDetect0      = 0x00000000;
  gpio[index]->gpioLvlDetect1      = 0x00000000;
  gpio[index]->gpioRisingDetect    = 0x00000000;
  gpio[index]->gpioFallingDetect   = 0x00000000;
  gpio[index]->gpioDebounceEnable  = 0x00000000;
  gpio[index]->gpioDebouncingTime  = 0x00000000;
}

void storeGpio(device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress, u32int value)
{
  DEBUG(VP_OMAP_35XX_GPIO, "%s store to pAddr: %.8x, vAddr: %.8x, access size: %x, val %.8x" EOL,
      dev->deviceName, physicalAddress, virtualAddress, (u32int)size, value);

  s32int index = getIndexByAddress(physicalAddress);
  u32int regOffset = physicalAddress & ~ADDRESS_MASK;
  if (index < 0)
  {
    DIE_NOW(NULL, "cannot translate physical address to GPIO number");
  }

  if (gpio[index]->physicalId < 0)
  {
    /*
     * This is a virtual GPIO without physical counterpart.
     */
    switch (regOffset)
    {
      case GPIO_REVISION:
      case GPIO_SYSSTATUS:
      case GPIO_DATAIN:
        DEBUG(VP_OMAP_35XX_GPIO, "GPIO: ignoring store to read-only register");
        break;
      case GPIO_SYSCONFIG:
        if ((value & GPIO_SYSCONFIG_SOFTRESET) == GPIO_SYSCONFIG_SOFTRESET)
        {
          DEBUG(VP_OMAP_35XX_GPIO, "GPIO: soft reset" EOL);
          reset(index);
        }
        gpio[index]->gpioSysConfig = (value & ~(GPIO_SYSCONFIG_RESERVED | GPIO_SYSCONFIG_SOFTRESET));
        break;
      case GPIO_IRQSTATUS1:
        if (value != 0xffffffff)
        {
          DIE_NOW(NULL, "clearing random interrupts 1");
        }
        gpio[index]->gpioIrqStatus1 = gpio[index]->gpioIrqStatus1 & ~value;
        break;
      case GPIO_IRQENABLE1:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling interrupt 1");
        }
        gpio[index]->gpioIrqEnable1 = value;
        break;
      case GPIO_IRQSTATUS2:
        if (value != 0xffffffff)
        {
          DIE_NOW(NULL, "clearing random interrupts 2");
        }
        gpio[index]->gpioIrqStatus2 = gpio[index]->gpioIrqStatus2 & ~value;
        break;
      case GPIO_IRQENABLE2:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling interrupt 2");
        }
        gpio[index]->gpioIrqEnable2 = value;
        break;
      case GPIO_CTRL:
        if ((value & GPIO_CTRL_DISABLEMOD) == GPIO_CTRL_DISABLEMOD)
        {
          DIE_NOW(NULL, "disabling module! investigate.");
        }
        gpio[index]->gpioCtrl = value & ~GPIO_CTRL_RESERVED;
        break;
      case GPIO_OE:
        printf("Warning: guest enabling output on disconnected GPIO" EOL);
        gpio[index]->gpioOE = value;
        break;
      case GPIO_DATAOUT:
        printf("Warning: guest writing to disconnected GPIO" EOL);
        gpio[index]->gpioDataOut = value;
        break;
      case GPIO_LEVELDETECT0:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling low-level detection");
        }
        gpio[index]->gpioLvlDetect0 = value;
        break;
      case GPIO_LEVELDETECT1:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling high-level detection");
        }
        gpio[index]->gpioLvlDetect1 = value;
        break;
      case GPIO_RISINGDETECT:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling rising-edge detection");
        }
        gpio[index]->gpioRisingDetect = value;
        break;
      case GPIO_FALLINGDETECT:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling falling-edge detection");
        }
        gpio[index]->gpioFallingDetect = value;
        break;
      case GPIO_CLEARIRQENABLE1:
        if ((gpio[index]->gpioIrqEnable1 & value))
        {
          DIE_NOW(NULL, "clearing interrupt 1");
          gpio[index]->gpioIrqEnable1 &= ~value;
        }
        break;
      case GPIO_SETIRQENABLE1:
        if ((gpio[index]->gpioIrqEnable1 | value) != value)
        {
          DIE_NOW(NULL, "enabling interrupt 1");
          gpio[index]->gpioIrqEnable1 |= value;
        }
        break;
      case GPIO_CLEARIRQENABLE2:
        if ((gpio[index]->gpioIrqEnable2 & value))
        {
          DIE_NOW(NULL, "clearing interrupt 2");
          gpio[index]->gpioIrqEnable2 &= ~value;
        }
        break;
      case GPIO_SETIRQENABLE2:
        if ((gpio[index]->gpioIrqEnable2 | value) != value)
        {
          DIE_NOW(NULL, "enabling interrupt 2");
          gpio[index]->gpioIrqEnable2 |= value;
        }
        break;
      case GPIO_CLEARWKUENA:
        if ((gpio[index]->gpioWakeupEnable & value))
        {
          DIE_NOW(NULL, "clearing wake-up enable");
          gpio[index]->gpioIrqEnable2 &= ~value;
        }
        break;
      case GPIO_CLEARDATAOUT:
        printf("Warning: guest writing to disconnected GPIO" EOL);
        gpio[index]->gpioDataOut &= ~value;
        break;
      case GPIO_SETDATAOUT:
        printf("Warning: guest writing to disconnected GPIO" EOL);
        gpio[index]->gpioDataOut |= value;
        break;
      case GPIO_WAKEUPENABLE:
      case GPIO_DEBOUNCENABLE:
      case GPIO_DEBOUNCINGTIME:
      case GPIO_SETWKUENA:
        DIE_NOW(NULL, "unimplemented store to register of disconnected GPIO");
      default:
        DIE_NOW(NULL, "store to invalid register of disconnected GPIO");
    }
  }
  else
  {
    /*
     * This GPIO is mapped to a physical GPIO.
     */
    switch (regOffset)
    {
      case GPIO_REVISION:
      case GPIO_SYSSTATUS:
      case GPIO_DATAIN:
        DEBUG(VP_OMAP_35XX_GPIO, "GPIO: ignoring store to read-only register");
        break;
      case GPIO_SYSCONFIG:
      case GPIO_CTRL:
      case GPIO_OE:
      case GPIO_DATAOUT:
      case GPIO_CLEARDATAOUT:
      case GPIO_SETDATAOUT:
        beStoreGPIO(regOffset, value, gpio[index]->physicalId);
        break;
      case GPIO_IRQSTATUS1:
        if (value != 0xffffffff)
        {
          DIE_NOW(NULL, "clearing random interrupts 1");
        }
        beStoreGPIO(regOffset, value, gpio[index]->physicalId);
        break;
      case GPIO_IRQENABLE1:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling interrupt 1");
        }
        beStoreGPIO(regOffset, value, gpio[index]->physicalId);
        break;
      case GPIO_WAKEUPENABLE:
      case GPIO_IRQSTATUS2:
      case GPIO_IRQENABLE2:
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
        DIE_NOW(NULL, "unimplemented store to register of connected GPIO");
        break;
      default:
        DIE_NOW(NULL, "store to invalid register of connected GPIO");
    }
  }
}

