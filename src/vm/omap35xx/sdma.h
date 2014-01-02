#ifndef __VM__OMAP35XX__SDMA_H__
#define __VM__OMAP35XX__SDMA_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"

#define SDMA_CHANNEL_COUNT   32

typedef struct SdmaIndexedRegs
{
  u32int ccr;
  u32int clnkCtrl;
  u32int cicr;
  u32int csr;
  u32int csdp;
  u32int cen;
  u32int cfn;
  u32int cssa;
  u32int cdsa;
  u32int csei;
  u32int csfi;
  u32int cdei;
  u32int cdfi;
  u32int csac;
  u32int cdac;
  u32int ccen;
  u32int ccfn;
  u32int color;
} ChannelRegisters;

struct Sdma
{
  u32int irqStatusL0;
  u32int irqStatusL1;
  u32int irqStatusL2;
  u32int irqStatusL3;
  u32int irqEnableL0;
  u32int irqEnableL1;
  u32int irqEnableL2;
  u32int irqEnableL3;
  u32int sysStatus;
  u32int ocpSysConfig;
  u32int caps0;
  u32int caps2;
  u32int caps3;
  u32int caps4;
  u32int gcr;
  ChannelRegisters chIndexedRegs[SDMA_CHANNEL_COUNT]; 
};


void initSdma(virtualMachine *vm) __cold__;
u32int loadSdma(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSdma(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);
void sdmaThrowInterrupt(GCONTXT *context, u32int dmaChannel);



#endif /* __VM__OMAP35XX__SDMA_H__ */
