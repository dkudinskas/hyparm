#ifndef __HARDWARE__SDMA_H__
#define __HARDWARE__SDMA_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


#define SDMA_REVISION_NUMBER      0x40


// register offsets and definitions
#define SDMA_REVISION        0x00000000 // revision number, R/O
#define SDMA_IRQSTATUS_L0    0x00000008 // irq status on line 0, R/W
#define SDMA_IRQSTATUS_L1    0x0000000C // irq status on line 1, R/W
#define SDMA_IRQSTATUS_L2    0x00000010 // irq status on line 2, R/W
#define SDMA_IRQSTATUS_L3    0x00000014 // irq status on line 3, R/W
#define SDMA_IRQENABLE_L0    0x00000018 // irq enable on line 0, R/W
#define SDMA_IRQENABLE_L1    0x0000001C // irq enable on line 1, R/W
#define SDMA_IRQENABLE_L2    0x00000020 // irq enable on line 2, R/W
#define SDMA_IRQENABLE_L3    0x00000024 // irq enable on line 3, R/W
#define SDMA_SYSSTATUS       0x00000028 // system status, R/O
#define SDMA_OCP_SYSCONFIG   0x0000002C // system config reg, R/W
#define SDMA_CAPS_0          0x00000064 // DMA capabilities register, R/O
#define SDMA_CAPS_2          0x0000006C // DMA capabilities register, R/O
#define SDMA_CAPS_3          0x00000070 // DMA capabilities register, R/O
#define SDMA_CAPS_4          0x00000074 // DMA capabilities register, R/O
#define SDMA_GCR             0x00000078 // high/low priority channel fifo sharing
// indexed stuff ...
#define SDMA_CCRi            0x00000080 //+(i*0x60) channel control register, R/W
#define SDMA_CCRi_RESERVED         0x00000800
#define SDMA_CCRi_WR_ACTIVE        0x00000400
#define SDMA_CCRi_RD_ACTIVE        0x00000200
#define SDMA_CCRi_SUSPEND_SENS     0x00000100
#define SDMA_CCRi_ENABLE           0x00000080
#define SDMA_CCRi_READ_PRIORITY    0x00000040
#define SDMA_CCRi_FRAME_SYNC       0x00000020
#define SDMA_CCRi_SYNC_CTRL        0x0000001F
#define SDMA_CLNK_CTRLi      0x00000084 //+(i*0x60) channel link control reg, R/W
#define SDMA_CICRi           0x00000088 //+(i*0x60) channel irq control reg, R/W
#define SDMA_CSRi            0x0000008C //+(i*0x60) channel status register, R/W
#define SDMA_CSDPi           0x00000090 //+(i*0x60) channel src/dst params, R/W
#define SDMA_CENi            0x00000094 //+(i*0x60) channel element number, R/O
#define SDMA_CFNi            0x00000098 //+(i*0x60) channel frame number, R/O
#define SDMA_CSSAi           0x0000009C //+(i*0x60) channel src start addr, R/W
#define SDMA_CDSAi           0x000000A0 //+(i*0x60) channel dest start addr, R/W
#define SDMA_CSEIi           0x000000A4 //+(i*0x60) channel src elem index, R/W
#define SDMA_CSFIi           0x000000A8 //+(i*0x60) channel src frame index, R/W
#define SDMA_CDEIi           0x000000AC //+(i*0x60) channel dest elem index, R/W
#define SDMA_CDFIi           0x000000B0 //+(i*0x60) channel dest frame index, R/W
#define SDMA_CSACi           0x000000B4 //+(i*0x60) channel src addr value, R/O
#define SDMA_CDACi           0x000000B8 //+(i*0x60) channel dest addr value, R/O
#define SDMA_CCENi           0x000000BC //+(i*0x60) channel curr x-ered elem nr in frame, R/W
#define SDMA_CCFNi           0x000000C0 //+(i*0x60) channel curr x-ered frame in x-fer, R/W
#define SDMA_COLORi          0x000000C4 //+(i*0x60) channel DMA color key


void initSdma(void);

void resetSdma(void);

u32int loadSdma(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSdma(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);


struct SdmaIndexedRegs
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
};

typedef struct SdmaIndexedRegs ChannelRegisters;

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
  ChannelRegisters chIndexedRegs[32]; 
};

#endif
