#ifndef __SDMA_H__
#define __SDMA_H__

#include "types.h"
#include "serial.h"
#include "hardwareLibrary.h"

#define SDMA_REVISION_NUMBER      0x40

// uncomment me to enable debug : 
#define SDMA_DBG

// register offsets and definitions
#define SDMA_REVISION        0x00000000 // revision number, R/O
#define SDMA_IRQSTATUS_Lj    0x00000008 //+(j*0x4), irq status on line 0..3, R/W
#define SDMA_IRQENABLE_Lj    0x00000018 //+(j*0x4), irq enable on line 0..3, R/W
#define SDMA_SYSSTATUS       0x00000028 // system status, R/O
#define SDMA_OCP_SYSCONFIG   0x0000002C // system config reg, R/W
#define SDMA_CAPS_0          0x00000064 // DMA capabilities register, R/O
#define SDMA_CAPS_2          0x0000006C // DMA capabilities register, R/O
#define SDMA_CAPS_3          0x00000070 // DMA capabilities register, R/O
#define SDMA_CAPS_4          0x00000074 // DMA capabilities register, R/O
#define SDMA_GCR             0x00000078 // high/low priority channel fifo sharing
#define SDMA_CCRi            0x00000080 //+(i*0x60) channel control register, R/W
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

u32int loadSdma(device * dev, ACCESS_SIZE size, u32int address);

void storeSdma(device * dev, ACCESS_SIZE size, u32int address, u32int value);


struct Sdma
{
  u32int temp;
};

#endif
