#ifndef __VM__OMAP35XX__OMAP35XX_H__
#define __VM__OMAP35XX__OMAP35XX_H__


struct EmulatedVirtualMachine
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
  struct Mmc *mmc[3];
  struct PmRt     *pmrt;
  struct PmGpmc   *pmgpmc;
  struct PmOcmRam *pmocmram;
  struct PmOcmRom *pmocmrom;
  struct PmIva    *pmiva;
  struct Sdrc *sdrc;
  struct Sms *sms;
  struct WatchdogTimer *wdtimer2;
};

#endif
