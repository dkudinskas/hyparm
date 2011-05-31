#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"
#include "drivers/beagle/beGPIO.h"

#include "vm/omap35xx/gpio.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()

extern GCONTXT * getGuestContext(void);

struct Gpio * gpio[6];

void initGpio(u32int gpioNumber)
{
  gpio[gpioNumber-1] = (struct Gpio*)mallocBytes(sizeof(struct Gpio));
  if (gpio[gpioNumber-1] == 0)
  {
    DIE_NOW(0, "Failed to allocate Gpio.");
  }
  else
  {
    memset((void*)gpio[gpioNumber-1], 0x0, sizeof(struct Gpio));
#ifdef GPIO_DBG
    printf("Initializing GPIO%x at %08x\n", gpioNumber, (u32int)gpio[gpioNumber-1]);
#endif
  }
  
  resetGpio(gpioNumber);
}

void resetGpio(u32int num)
{
  // reset registers to default values
  gpio[num-1]->gpioRevision        = 0x00000025;
  gpio[num-1]->gpioSysConfig       = 0x00000000;
  gpio[num-1]->gpioSysStatus       = 0x00000001;
  gpio[num-1]->gpioIrqStatus1      = 0x00000000;
  gpio[num-1]->gpioIrqEnable1      = 0x00000000;
  gpio[num-1]->gpioWakeupEnable    = 0x00000000;
  gpio[num-1]->gpioIrqStatus2      = 0x00000000;
  gpio[num-1]->gpioIrqEnable2      = 0x00000000;
  gpio[num-1]->gpioCtrl            = 0x00000002;
  gpio[num-1]->gpioOE              = 0x00000000;
  gpio[num-1]->gpioDataIn          = 0x00000000;
  gpio[num-1]->gpioDataOut         = 0x00000000;
  gpio[num-1]->gpioLvlDetect0      = 0x00000000;
  gpio[num-1]->gpioLvlDetect1      = 0x00000000;
  gpio[num-1]->gpioRisingDetect    = 0x00000000;
  gpio[num-1]->gpioFallingDetect   = 0x00000000;
  gpio[num-1]->gpioDebounceEnable  = 0x00000000;
  gpio[num-1]->gpioDebouncingTime  = 0x00000000;
  gpio[num-1]->gpioClearIrqEnable1 = 0x00000000;
  gpio[num-1]->gpioSetIrqEnable1   = 0x00000000;
  gpio[num-1]->gpioClearIrqEnable2 = 0x00000000;
  gpio[num-1]->gpioSetIrqEnable2   = 0x00000000;
  gpio[num-1]->gpioClearWkuEnable  = 0x00000000;
  gpio[num-1]->gpioSetWkuEnable    = 0x00000000;
  gpio[num-1]->gpioClearDataOut    = 0x00000000;
  gpio[num-1]->gpioSetDataOut      = 0x00000000;
  
  // set reset complete bit
  gpio[num-1]->gpioSysStatus = GPIO_SYSSTATUS_RESETDONE;
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
      gpioNum = 0;
      regOffset = phyAddr - GPIO1;
      break;
    case GPIO2:
      gpioNum = 1;
      regOffset = phyAddr - GPIO2;
      break;
    case GPIO3:
      gpioNum = 2;
      regOffset = phyAddr - GPIO3;
      break;
    case GPIO4:
      gpioNum = 3;
      regOffset = phyAddr - GPIO4;
      break;
    case GPIO5:
      gpioNum = 4;
      regOffset = phyAddr - GPIO5;
      break;
    case GPIO6:
      gpioNum = 5;
      regOffset = phyAddr - GPIO6;
      break;
    default:
      DIE_NOW(0, "GPIO: loadGpio - invalid gpio number for base address");
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
	{
		/* FreeRTOS GPIO status */
	    val = gpio[gpioNum]->gpioDataOut;
		if(gpioNum==4 || gpioNum==5)
		{
			val=beGetGPIO(regOffset,gpioNum);
		}
		break;
	}
    case GPIO_CLEARDATAOUT:
	{
		val = gpio[gpioNum]->gpioClearDataOut;
		if(gpioNum==4 || gpioNum==5)
		{
			val = beGetGPIO(regOffset,gpioNum);
		}
		break;
	}
    case GPIO_SETDATAOUT:
	{
		val = gpio[gpioNum]->gpioSetDataOut;
		if(gpioNum==4 || gpioNum==5)
		{
			val = beGetGPIO(regOffset,gpioNum);
		}
		break;
	}
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
      printf("GPIO: load from unimplemented register %x\n", regOffset);
      DIE_NOW(0, "panic.");
      break;
    default:
      DIE_NOW(0, "Gpio: load on invalid register.");
  }
#ifdef GPIO_DBG
  printf(dev->deviceName);
  printf(" load from pAddr: %08x, vAddr: %08x access size %x val = %08x\n",
         phyAddr, address, (u32int)size, val);
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
  printf(dev->deviceName);
  printf(" store to pAddr: %08x, vAddr: %08x, access size: %x, val %08x\n",
        phyAddr, address, (u32int)size, value);
#endif

  u32int regOffset = 0;
  u32int gpioNum = 0;
  switch (phyAddr & 0xFFFFF000)
  {
    case GPIO1:
      gpioNum = 0;
      regOffset = phyAddr - GPIO1;
      break;
    case GPIO2:
      gpioNum = 1;
      regOffset = phyAddr - GPIO2;
      break;
    case GPIO3:
      gpioNum = 2;
      regOffset = phyAddr - GPIO3;
      break;
    case GPIO4:
      gpioNum = 3;
      regOffset = phyAddr - GPIO4;
      break;
    case GPIO5:
      gpioNum = 4;
      regOffset = phyAddr - GPIO5;
      break;
    case GPIO6:
      gpioNum = 5;
      regOffset = phyAddr - GPIO6;
      break;
    default:
      DIE_NOW(0, "GPIO: storeGpio - invalid gpio number for base address");
  }

  switch (regOffset)
  {
    case GPIO_REVISION:
      DIE_NOW(0, "GPIO: storing to a R/O reg (revision)");
      break;
    case GPIO_SYSCONFIG:
      if ((value & GPIO_SYSCONFIG_SOFTRESET) == GPIO_SYSCONFIG_SOFTRESET)
      {
#ifdef GPIO_DBG
        printf("GPIO: soft reset.\n");
#endif
        resetGpio(gpioNum);
      }
      else
      {
        gpio[gpioNum]->gpioSysConfig = (value & ~GPIO_SYSCONFIG_RESERVED);
      }
      break;
    case GPIO_SYSSTATUS:
      DIE_NOW(0, "GPIO: storing to a R/O reg (sysStatus)");
      break;
    case GPIO_IRQSTATUS1:
      if (value != 0xffffffff)
      {
        DIE_NOW(0, "GPIO: clearing random interrupts. have a look!...");
      }
      gpio[gpioNum]->gpioIrqStatus1 = gpio[gpioNum]->gpioIrqStatus1 & ~value;
      break;
    case GPIO_IRQENABLE1:
      if (value != 0)
      {
        DIE_NOW(0, "GPIO: enabling interrupt! have a look at this...");
      }
      gpio[gpioNum]->gpioIrqEnable1 = value;
      break;
    case GPIO_IRQSTATUS2:
      if (value != 0xffffffff)
      {
        DIE_NOW(0, "GPIO: clearing random interrupts. have a look!...");
      }
      gpio[gpioNum]->gpioIrqStatus2 = gpio[gpioNum]->gpioIrqStatus2 & ~value;
      break;
    case GPIO_IRQENABLE2:
      if (value != 0)
      {
        DIE_NOW(0, "GPIO: enabling interrupt! have a look at this...");
      }
      gpio[gpioNum]->gpioIrqEnable2 = value;
      break;
    case GPIO_CTRL:
      if ((value & GPIO_CTRL_DISABLEMOD) == GPIO_CTRL_DISABLEMOD)
      {
        DIE_NOW(0, "GPIO: disabling module! investigate.");
      }
      gpio[gpioNum]->gpioCtrl = value & ~GPIO_CTRL_RESERVED;
      break;
    case GPIO_OE:
      {
	  	/* value can be any 32-bit number */
      	gpio[gpioNum]->gpioOE = value;
	  	/* FreeRTOS initialization */
	  	if(gpioNum==4 || gpioNum==5)
	  	{
	  		beStoreGPIO(regOffset,value,gpioNum);
	  	}
      break;
	}
    case GPIO_CLEARDATAOUT:
      {
	  	/* value can be any 32-bit number */
      	gpio[gpioNum]->gpioClearDataOut=value;
		gpio[gpioNum]->gpioDataOut=~(value);
	  	/* FreeRTOS initialization */
	  	if(gpioNum==4 || gpioNum==5)
	  	{
	  		beStoreGPIO(regOffset,value,gpioNum);
	  	}
      	break;
	}
    case GPIO_SETDATAOUT:
      {
	  	/* value can be any 32-bit number */
      	gpio[gpioNum]->gpioSetDataOut=value;
		gpio[gpioNum]->gpioDataOut=value;
	  	/* FreeRTOS initialization */
	  	if(gpioNum==4 || gpioNum==5)
	  	{
	  		beStoreGPIO(regOffset,value,gpioNum);
	  	}
      	break;
	}
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
      printf("GPIO: store to unimplemented register %x\n", regOffset);
      DIE_NOW(gc, "panic.");
      break;
    default:
      DIE_NOW(gc, "Gpio: store to invalid register.");
  }
}

