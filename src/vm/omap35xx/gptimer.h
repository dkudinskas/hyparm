#ifndef __VM__OMAP_35XX__GPTIMER_H__
#define __VM__OMAP_35XX__GPTIMER_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct GeneralPurposeTimer
{
  // registers
  u32int gptTiocpCfg;
  u32int gptTistat;
  u32int gptTisr;
  u32int gptTier;
  u32int gptTwer;
  u32int gptTclr;
  u32int gptTcrr;
  u32int gptTldr;
  u32int gptTtgr;
  u32int gptTwps;
  u32int gptTmar;
  u32int gptTcar1;
  u32int gptTsicr;
  u32int gptTcar2;
  u32int gptTpir;
  u32int gptTnir;
  u32int gptTcvr;
  u32int gptTocr;
  u32int gptTowr;
  // internal variables...
  u32int shadowValue;
};

void initGPTimer(virtualMachine *vm) __cold__;
u32int loadGPTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeGPTimer(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif

