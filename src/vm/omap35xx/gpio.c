#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "drivers/beagle/beGPIO.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/gpio.h"


#define ADDRESS_MASK  0xFFFFF000

/************************
 * REGISTER DEFINITIONS *
 ************************/
#define GPIO_REVISION            0x000

#define GPIO_SYSCONFIG           0x010
#define GPIO_SYSCONFIG_RESERVED     0xFFFFFFE0
#define GPIO_SYSCONFIG_IDLEMODE     0x00000018
#define GPIO_SYSCONFIG_ENAWAKEUP    0x00000004
#define GPIO_SYSCONFIG_SOFTRESET    0x00000002
#define GPIO_SYSCONFIG_AUTOIDLE     0x00000001

#define GPIO_SYSSTATUS           0x014
#define GPIO_SYSSTATUS_RESERVED     0xFFFFFFFE
#define GPIO_SYSSTATUS_RESETDONE    0x00000001

#define GPIO_IRQSTATUS1          0x018
#define GPIO_IRQENABLE1          0x01C
#define GPIO_WAKEUPENABLE        0x020
#define GPIO_IRQSTATUS2          0x028
#define GPIO_IRQENABLE2          0x02C

#define GPIO_CTRL                0x030
#define GPIO_CTRL_RESERVED          0xFFFFFFF8
#define GPIO_CTRL_GATERATIO         0x00000006
#define GPIO_CTRL_DISABLEMOD        0x00000001

#define GPIO_OE                  0x034
#define GPIO_DATAIN              0x038
#define GPIO_DATAOUT             0x03C
#define GPIO_LEVELDETECT0        0x040
#define GPIO_LEVELDETECT1        0x044
#define GPIO_RISINGDETECT        0x048
#define GPIO_FALLINGDETECT       0x04C
#define GPIO_DEBOUNCENABLE       0x050
#define GPIO_DEBOUNCINGTIME      0x054
#define GPIO_CLEARIRQENABLE1     0x060
#define GPIO_SETIRQENABLE1       0x064
#define GPIO_CLEARIRQENABLE2     0x070
#define GPIO_SETIRQENABLE2       0x074
#define GPIO_CLEARWKUENA         0x080
#define GPIO_SETWKUENA           0x084
#define GPIO_CLEARDATAOUT        0x090
#define GPIO_SETDATAOUT          0x094


static inline s32int getIndexByAddress(u32int physicalAddress);
static void reset(struct Gpio *gpio);


void connectGpio(virtualMachine *vm, u32int gpioNumber, u32int physicalGpioNumber)
{
  vm->gpio[gpioNumber - 1]->physicalId = physicalGpioNumber - 1;
}

void initGpio(virtualMachine *vm, u32int gpioNumber)
{
  const s32int index = gpioNumber - 1;
  vm->gpio[index] = (struct Gpio *)calloc(1, sizeof(struct Gpio));
  if (vm->gpio[index] == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate Gpio.");
  }

  DEBUG(VP_OMAP_35XX_GPIO, "initGpio: GPIO%x @ %p" EOL, gpioNumber, vm->gpio[index]);
  vm->gpio[index]->physicalId = -1;
  reset(vm->gpio[index]);
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
u32int loadGpio(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress)
{
  const s32int index = getIndexByAddress(physicalAddress);
  u32int regOffset = physicalAddress & ~ADDRESS_MASK;
  if (index < 0)
  {
    DIE_NOW(NULL, "cannot translate physical address to GPIO number");
  }

  struct Gpio *gpio = context->vm.gpio[index];
  u32int val = 0;
  if (gpio->physicalId < 0)
  {
    /*
     * This is a virtual GPIO without physical counterpart.
     */
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
      case GPIO_CLEARIRQENABLE1:
      case GPIO_SETIRQENABLE1:
        val = gpio->gpioIrqEnable1;
        break;
      case GPIO_WAKEUPENABLE:
      case GPIO_CLEARWKUENA:
      case GPIO_SETWKUENA:
        val = gpio->gpioWakeupEnable;
        break;
      case GPIO_IRQSTATUS2:
        val = gpio->gpioIrqStatus2;
        break;
      case GPIO_IRQENABLE2:
      case GPIO_CLEARIRQENABLE2:
      case GPIO_SETIRQENABLE2:
        val = gpio->gpioIrqEnable2;
        break;
      case GPIO_CTRL:
        val = gpio->gpioCtrl;
        break;
      case GPIO_OE:
        val = gpio->gpioOE;
        break;
      case GPIO_DATAIN:
        printf("Warning: guest reading from disconnected GPIO" EOL);
        val = gpio->gpioDataIn;
        break;
      case GPIO_DATAOUT:
      case GPIO_CLEARDATAOUT:
      case GPIO_SETDATAOUT:
        val = gpio->gpioDataOut;
        break;
      case GPIO_LEVELDETECT0:
        val = gpio->gpioLvlDetect0;
        break;
      case GPIO_LEVELDETECT1:
        val = gpio->gpioLvlDetect1;
        break;
      case GPIO_RISINGDETECT:
        val = gpio->gpioRisingDetect;
        break;
      case GPIO_FALLINGDETECT:
        val = gpio->gpioFallingDetect;
        break;
      case GPIO_DEBOUNCENABLE:
        val = gpio->gpioDebounceEnable;
        break;
      case GPIO_DEBOUNCINGTIME:
        val = gpio->gpioDebouncingTime;
        break;
      default:
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
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
        val = beGetGPIO(regOffset, gpio->physicalId);
        break;
      default:
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_GPIO, "%s load from pAddr: %.8x, vAddr: %.8x access size %x val = %.8x" EOL,
      dev->deviceName, physicalAddress, virtualAddress, (u32int)size, val);
  return val;
}

static void reset(struct Gpio *gpio)
{
  /*
   * Reset registers to default values
   * WARNING: DO NOT CLEAR mapping to physical GPIO
   */
  gpio->gpioRevision        = 0x00000025;
  gpio->gpioSysConfig       = 0x00000000;
  gpio->gpioSysStatus       = 0x00000001;
  gpio->gpioIrqStatus1      = 0x00000000;
  gpio->gpioIrqEnable1      = 0x00000000;
  gpio->gpioWakeupEnable    = 0x00000000;
  gpio->gpioIrqStatus2      = 0x00000000;
  gpio->gpioIrqEnable2      = 0x00000000;
  gpio->gpioCtrl            = 0x00000002;
  gpio->gpioOE              = 0xFFFFFFFF;
  gpio->gpioDataIn          = 0x00000000;
  gpio->gpioDataOut         = 0x00000000;
  gpio->gpioLvlDetect0      = 0x00000000;
  gpio->gpioLvlDetect1      = 0x00000000;
  gpio->gpioRisingDetect    = 0x00000000;
  gpio->gpioFallingDetect   = 0x00000000;
  gpio->gpioDebounceEnable  = 0x00000000;
  gpio->gpioDebouncingTime  = 0x00000000;
}

void storeGpio(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress, u32int value)
{
  DEBUG(VP_OMAP_35XX_GPIO, "%s store to pAddr: %.8x, vAddr: %.8x, access size: %x, val %.8x" EOL,
      dev->deviceName, physicalAddress, virtualAddress, (u32int)size, value);

  s32int index = getIndexByAddress(physicalAddress);
  u32int regOffset = physicalAddress & ~ADDRESS_MASK;
  if (index < 0)
  {
    DIE_NOW(NULL, "cannot translate physical address to GPIO number");
  }

  struct Gpio *gpio = context->vm.gpio[index];
  if (gpio->physicalId < 0)
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
          reset(gpio);
        }
        gpio->gpioSysConfig = (value & ~(GPIO_SYSCONFIG_RESERVED | GPIO_SYSCONFIG_SOFTRESET));
        break;
      case GPIO_IRQSTATUS1:
        if (value != 0xffffffff)
        {
          DIE_NOW(NULL, "clearing random interrupts 1");
        }
        gpio->gpioIrqStatus1 = gpio->gpioIrqStatus1 & ~value;
        break;
      case GPIO_IRQENABLE1:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling interrupt 1");
        }
        gpio->gpioIrqEnable1 = value;
        break;
      case GPIO_IRQSTATUS2:
        if (value != 0xffffffff)
        {
          DIE_NOW(NULL, "clearing random interrupts 2");
        }
        gpio->gpioIrqStatus2 = gpio->gpioIrqStatus2 & ~value;
        break;
      case GPIO_IRQENABLE2:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling interrupt 2");
        }
        gpio->gpioIrqEnable2 = value;
        break;
      case GPIO_CTRL:
        if ((value & GPIO_CTRL_DISABLEMOD) == GPIO_CTRL_DISABLEMOD)
        {
          DIE_NOW(NULL, "disabling module! investigate.");
        }
        gpio->gpioCtrl = value & ~GPIO_CTRL_RESERVED;
        break;
      case GPIO_OE:
        printf("Warning: guest enabling output on disconnected GPIO" EOL);
        gpio->gpioOE = value;
        break;
      case GPIO_DATAOUT:
        printf("Warning: guest writing to disconnected GPIO" EOL);
        gpio->gpioDataOut = value;
        break;
      case GPIO_LEVELDETECT0:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling low-level detection");
        }
        gpio->gpioLvlDetect0 = value;
        break;
      case GPIO_LEVELDETECT1:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling high-level detection");
        }
        gpio->gpioLvlDetect1 = value;
        break;
      case GPIO_RISINGDETECT:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling rising-edge detection");
        }
        gpio->gpioRisingDetect = value;
        break;
      case GPIO_FALLINGDETECT:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling falling-edge detection");
        }
        gpio->gpioFallingDetect = value;
        break;
      case GPIO_DEBOUNCENABLE:
      {
        if (gpio->gpioDebounceEnable == value)
        {
          printf("%s: unimplemented store to gpioDebounceEnable" EOL, __func__);
        }
        break;
      }
      case GPIO_CLEARIRQENABLE1:
        if ((gpio->gpioIrqEnable1 & value))
        {
          DIE_NOW(NULL, "clearing interrupt 1");
          gpio->gpioIrqEnable1 &= ~value;
        }
        break;
      case GPIO_SETIRQENABLE1:
        if ((gpio->gpioIrqEnable1 | value) != value)
        {
          DIE_NOW(NULL, "enabling interrupt 1");
          gpio->gpioIrqEnable1 |= value;
        }
        break;
      case GPIO_CLEARIRQENABLE2:
        if ((gpio->gpioIrqEnable2 & value))
        {
          DIE_NOW(NULL, "clearing interrupt 2");
          gpio->gpioIrqEnable2 &= ~value;
        }
        break;
      case GPIO_SETIRQENABLE2:
        if ((gpio->gpioIrqEnable2 | value) != value)
        {
          DIE_NOW(NULL, "enabling interrupt 2");
          gpio->gpioIrqEnable2 |= value;
        }
        break;
      case GPIO_CLEARWKUENA:
        if ((gpio->gpioWakeupEnable & value))
        {
          DIE_NOW(NULL, "clearing wake-up enable");
          gpio->gpioIrqEnable2 &= ~value;
        }
        break;
      case GPIO_CLEARDATAOUT:
        printf("Warning: guest writing to disconnected GPIO" EOL);
        gpio->gpioDataOut &= ~value;
        break;
      case GPIO_SETDATAOUT:
        printf("Warning: guest writing to disconnected GPIO" EOL);
        gpio->gpioDataOut |= value;
        break;
      case GPIO_WAKEUPENABLE:
      case GPIO_DEBOUNCINGTIME:
      case GPIO_SETWKUENA:
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      default:
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
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
        beStoreGPIO(regOffset, value, gpio->physicalId);
        break;
      case GPIO_IRQSTATUS1:
        if (value != 0xffffffff)
        {
          DIE_NOW(NULL, "clearing random interrupts 1");
        }
        beStoreGPIO(regOffset, value, gpio->physicalId);
        break;
      case GPIO_IRQENABLE1:
        if (value != 0)
        {
          DIE_NOW(NULL, "enabling interrupt 1");
        }
        beStoreGPIO(regOffset, value, gpio->physicalId);
        break;
      case GPIO_DEBOUNCENABLE:
      {
#ifdef CONFIG_GUEST_ANDROID
        beStoreGPIO(regOffset, value, gpio->physicalId);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
        break;
      }
      case GPIO_WAKEUPENABLE:
      case GPIO_IRQSTATUS2:
      case GPIO_IRQENABLE2:
      case GPIO_LEVELDETECT0:
      case GPIO_LEVELDETECT1:
      case GPIO_RISINGDETECT:
      case GPIO_FALLINGDETECT:
      case GPIO_DEBOUNCINGTIME:
      case GPIO_CLEARIRQENABLE1:
      case GPIO_SETIRQENABLE1:
      case GPIO_CLEARIRQENABLE2:
      case GPIO_SETIRQENABLE2:
      case GPIO_CLEARWKUENA:
      case GPIO_SETWKUENA:
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
        break;
      default:
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

