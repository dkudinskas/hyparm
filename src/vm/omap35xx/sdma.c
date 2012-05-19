#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdma.h"
#include "vm/omap35xx/timer32k.h"


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


static inline u32int getChannelNumber(u32int regOffs);
static void resetSdma(struct Sdma *sdma);


void initSdma(virtualMachine *vm)
{
  // init function: setup device, reset register values to defaults!
  struct Sdma *sdma = (struct Sdma *)calloc(1, sizeof(struct Sdma));
  if (sdma == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate sdma.");
  }
  vm->sdma = sdma;

  DEBUG(VP_OMAP_35XX_SDMA, "Initializing Sdma at %p size %#x" EOL, sdma, sizeof(struct Sdma));
  resetSdma(sdma);
}

static void resetSdma(struct Sdma *sdma)
{
  sdma->irqStatusL0      = 0x00000000;
  sdma->irqStatusL1      = 0x00000000;
  sdma->irqStatusL2      = 0x00000000;
  sdma->irqStatusL3      = 0x00000000;
  sdma->irqEnableL0      = 0x00000000;
  sdma->irqEnableL1      = 0x00000000;
  sdma->irqEnableL2      = 0x00000000;
  sdma->irqEnableL3      = 0x00000000;
  sdma->sysStatus        = 0x00000001;
  sdma->ocpSysConfig     = 0x00000000;
  sdma->caps0            = 0x000C0000;
  sdma->caps2            = 0x000001ff;
  sdma->caps3            = 0x000000f3;
  sdma->caps4            = 0x00003ffe;
  sdma->gcr              = 0x00010010;
  // init indexed registers
  int i = 0;
  for (i = 0; i < 32; i++)
  {
    sdma->chIndexedRegs[i].ccr       = 0x00000000;
    sdma->chIndexedRegs[i].clnkCtrl  = 0x00000000;
    sdma->chIndexedRegs[i].cicr      = 0x00000600;
    sdma->chIndexedRegs[i].csr       = 0x00000000;
    sdma->chIndexedRegs[i].csdp      = 0x00000000;
    sdma->chIndexedRegs[i].cen       = 0x00000000;
    sdma->chIndexedRegs[i].cfn       = 0x00000000;
    sdma->chIndexedRegs[i].cssa      = 0x00000000;
    sdma->chIndexedRegs[i].cdsa      = 0x00000000;
    sdma->chIndexedRegs[i].csei      = 0x00000000;
    sdma->chIndexedRegs[i].csfi      = 0x00000000;
    sdma->chIndexedRegs[i].cdei      = 0x00000000;
    sdma->chIndexedRegs[i].cdfi      = 0x00000000;
    sdma->chIndexedRegs[i].csac      = 0x00000000;
    sdma->chIndexedRegs[i].cdac      = 0x00000000;
    sdma->chIndexedRegs[i].ccen      = 0x00000000;
    sdma->chIndexedRegs[i].ccfn      = 0x00000000;
    sdma->chIndexedRegs[i].color     = 0x00000000;
  }
}


u32int loadSdma(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  struct Sdma* sdma = context->vm.sdma;

  u32int value = 0;
  u32int regOffs = phyAddr - SDMA;
  bool found = FALSE;
  switch (regOffs)
  {
    case SDMA_REVISION:
    {
      value = SDMA_REVISION_NUMBER;
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L0:
    {
      value = sdma->irqEnableL0;
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L1:
    {
      value = sdma->irqEnableL1;
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L2:
    {
      value = sdma->irqEnableL2;
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L3:
    {
      value = sdma->irqEnableL3;
      found = TRUE;
      break;
    }
    case SDMA_OCP_SYSCONFIG:
    {
      value = sdma->ocpSysConfig;
      found = TRUE;
      break;
    }
    case SDMA_IRQSTATUS_L0:
    case SDMA_IRQSTATUS_L1:
    case SDMA_IRQSTATUS_L2:
    case SDMA_IRQSTATUS_L3:
    case SDMA_SYSSTATUS:
    case SDMA_CAPS_0:
    case SDMA_CAPS_2:
    case SDMA_CAPS_3:
    case SDMA_CAPS_4:
    case SDMA_GCR:
      printf("loadSdma reg %#x" EOL, regOffs);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
  } // switch ends

  if (found)
  {
    DEBUG(VP_OMAP_35XX_SDMA, "%s: load from address %#.8x reg %#x value %#.8x" EOL, dev->deviceName,
        virtAddr, regOffs, value);
    return value;
  }

  // if we didnt hit the previous switch, maybe its one of the indexed registers
  u32int indexedRegOffs = (regOffs - 0x80) % 0x60;
  switch (indexedRegOffs + 0x80)
  {
    case SDMA_CCRi:
    case SDMA_CLNK_CTRLi:
    case SDMA_CICRi:
    case SDMA_CSRi:
    case SDMA_CSDPi:
    case SDMA_CENi:
    case SDMA_CFNi:
    case SDMA_CSSAi:
    case SDMA_CDSAi:
    case SDMA_CSEIi:
    case SDMA_CSFIi:
    case SDMA_CDEIi:
    case SDMA_CDFIi:
    case SDMA_CSACi:
    case SDMA_CDACi:
    case SDMA_CCENi:
    case SDMA_CCFNi:
    case SDMA_COLORi:
      printf("loadSdma indexed reg %#x reg %#x" EOL, indexedRegOffs + 0x80, regOffs);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
    default:
      printf("loadSdma indexed reg %#x reg %#x" EOL, indexedRegOffs + 0x80, regOffs);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}


void storeSdma(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  struct Sdma* sdma = context->vm.sdma;

  u32int regOffs = phyAddr - SDMA;
  bool found = FALSE;

  DEBUG(VP_OMAP_35XX_SDMA, "%s: store to address %.8x reg %.8x value %.8x" EOL, dev->deviceName,
      virtAddr, regOffs, value);
  switch (regOffs)
  {
    case SDMA_REVISION:
    {
      DIE_NOW(NULL, "SDMA storing to revision register (read only)");
      break;
    }
    case SDMA_GCR:
    {
      if (sdma->gcr != value)
      {
        DIE_NOW(NULL, "SDMA storing value to GCR!");
      }
      sdma->gcr = value;
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L0:
    {
      if (sdma->irqEnableL0 != value)
      {
        printf("%s: unimplemented store to irqEnableL0" EOL, __func__);
      }
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L1:
    {
      if (sdma->irqEnableL1 != value)
      {
        printf("%s: unimplemented store to irqEnableL1" EOL, __func__);
      }
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L2:
    {
      if (sdma->irqEnableL2 != value)
      {
        printf("%s: unimplemented store to irqEnableL2" EOL, __func__);
      }
      found = TRUE;
      break;
    }
    case SDMA_IRQENABLE_L3:
    {
      if (sdma->irqEnableL3 != value)
      {
        printf("%s: unimplemented store to irqEnableL3" EOL, __func__);
      }
      found = TRUE;
      break;
    }
    case SDMA_OCP_SYSCONFIG:
    {
      if (sdma->ocpSysConfig != value)
      {
        printf("%s: unimplemented store to ocpSysConfig" EOL, __func__);
      }
      found = TRUE;
      break;
    }
    case SDMA_IRQSTATUS_L0:
    case SDMA_IRQSTATUS_L1:
    case SDMA_IRQSTATUS_L2:
    case SDMA_IRQSTATUS_L3:
    case SDMA_SYSSTATUS:
    case SDMA_CAPS_0:
    case SDMA_CAPS_2:
    case SDMA_CAPS_3:
    case SDMA_CAPS_4:
      printf("storeSdma reg %x value %.8x" EOL, regOffs, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
  } // switch ends

  if (found)
  {
    return;
  }

  // if we didnt hit the previous switch, maybe its one of the indexed registers
  u32int channelIndex = getChannelNumber(regOffs);
  u32int indexedRegOffs = (regOffs - 0x80) % 0x60;
  switch (indexedRegOffs+0x80)
  {
    case SDMA_CCRi:
    {
      if ((value & SDMA_CCRi_ENABLE) == SDMA_CCRi_ENABLE)
      {
        printf("%s: Warning - enabling channel %x" EOL, dev->deviceName, channelIndex);
      }
      sdma->chIndexedRegs[channelIndex].ccr = value;
      break;
    }
    case SDMA_CLNK_CTRLi:
      sdma->chIndexedRegs[channelIndex].clnkCtrl = value;
      break;
    case SDMA_CICRi:
      sdma->chIndexedRegs[channelIndex].cicr = value;
      break;
    case SDMA_CSRi:
      sdma->chIndexedRegs[channelIndex].csr = value;
      break;
    case SDMA_CSDPi:
      sdma->chIndexedRegs[channelIndex].csdp = value;
      break;
    case SDMA_CENi:
      sdma->chIndexedRegs[channelIndex].cen = value;
      break;
    case SDMA_CFNi:
      sdma->chIndexedRegs[channelIndex].cfn = value;
      break;
    case SDMA_CSSAi:
      sdma->chIndexedRegs[channelIndex].cssa = value;
      break;
    case SDMA_CDSAi:
      sdma->chIndexedRegs[channelIndex].cdsa = value;
      break;
    case SDMA_CSEIi:
      sdma->chIndexedRegs[channelIndex].csei = value;
      break;
    case SDMA_CSFIi:
      sdma->chIndexedRegs[channelIndex].csfi = value;
      break;
    case SDMA_CDEIi:
      sdma->chIndexedRegs[channelIndex].cdei = value;
      break;
    case SDMA_CDFIi:
      sdma->chIndexedRegs[channelIndex].cdfi = value;
      break;
    case SDMA_CSACi:
      DEBUG(VP_OMAP_35XX_SDMA, "SDMA: WARNING: store to R/O register SDMA_CSACi" EOL);
      break; 
    case SDMA_CDACi:
      sdma->chIndexedRegs[channelIndex].cdac = value;
      break;
    case SDMA_CCENi:
      DEBUG(VP_OMAP_35XX_SDMA, "SDMA: WARNING: store to R/O register SDMA_CCENi" EOL);
      break; 
    case SDMA_CCFNi:
      DEBUG(VP_OMAP_35XX_SDMA, "SDMA: WARNING: store to R/O register SDMA_CCFNi" EOL);
      break; 
    case SDMA_COLORi:
      sdma->chIndexedRegs[channelIndex].color = value;
      break;
    default:
      printf("storeSdma indexed reg %x reg %x value %.8x" EOL, indexedRegOffs + 0x80, regOffs,
          value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static inline u32int getChannelNumber(u32int regOffs)
{
  return ((regOffs - 0x80) / 0x60);
}
