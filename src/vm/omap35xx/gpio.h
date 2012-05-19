#ifndef __VM__OMAP_35XX__GPIO_H__
#define __VM__OMAP_35XX__GPIO_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct Gpio
{
  u32int gpioRevision;
  u32int gpioSysConfig;
  u32int gpioSysStatus;
  u32int gpioIrqStatus1;
  u32int gpioIrqEnable1;
  u32int gpioWakeupEnable;
  u32int gpioIrqStatus2;
  u32int gpioIrqEnable2;
  u32int gpioCtrl;
  u32int gpioOE;
  u32int gpioDataIn;
  u32int gpioDataOut;
  u32int gpioLvlDetect0;
  u32int gpioLvlDetect1;
  u32int gpioRisingDetect;
  u32int gpioFallingDetect;
  u32int gpioDebounceEnable;
  u32int gpioDebouncingTime;
  s32int physicalId;
};


void connectGpio(virtualMachine *vm, u32int gpioNumber, u32int physicalGpioNumber) __cold__;

void initGpio(virtualMachine *vm, u32int gpioNumber) __cold__;

u32int loadGpio(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress);

void storeGpio(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress, u32int value);

#endif
