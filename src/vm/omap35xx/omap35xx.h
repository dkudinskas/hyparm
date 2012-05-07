#ifndef __HARDWARE__OMAP35XX_H__
#define __HARDWARE__OMAP35XX_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"

#include "guestManager/guestContext.h"

struct EmumulatedVirtualMachine;
typedef struct EmumulatedVirtualMachine virtualMachine;


struct EmumulatedVirtualMachine
{
  struct ClockManager* clockMan;
  struct Gpio* gpio[6];
  struct Gpmc* gpmc;
  struct GeneralPurposeTimer* gptimer;
  struct InterruptController* irqController;
  struct PowerAndResetManager* prMan;
  struct Sdma* sdma;
  struct SdramController* sdram;
  struct SystemControlModule* sysCtrlModule;
  struct Uart* uart[3];
  struct SynchronizedTimer32k* timer32k;
};

#endif
