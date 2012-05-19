#ifndef __VM__OMAP35XX__OMAP35XX_H__
#define __VM__OMAP35XX__OMAP35XX_H__


typedef struct EmumulatedVirtualMachine
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
} virtualMachine;

#endif
