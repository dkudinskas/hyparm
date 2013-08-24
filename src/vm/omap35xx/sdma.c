#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestExceptions.h"
#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdma.h"
#include "vm/omap35xx/sdmaInternals.h"
#include "vm/omap35xx/timer32k.h"
#include "vm/omap35xx/intc.h"
#ifndef CONFIG_NO_MMC
#include "vm/omap35xx/mmc.h"
#endif


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
    {
      value = sdma->irqStatusL0;
      found = TRUE;
      break;
    }
    case SDMA_IRQSTATUS_L1:
    {
      value = sdma->irqStatusL1;
      found = TRUE;
      break;
    }
    case SDMA_IRQSTATUS_L2:
    {
      value = sdma->irqStatusL2;
      found = TRUE;
      break;
    }
    case SDMA_IRQSTATUS_L3:
    {
      value = sdma->irqStatusL3;
      found = TRUE;
      break;
    }
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
  u32int channelIndex = getChannelNumber(regOffs);
  u32int indexedRegOffs = (regOffs - 0x80) % 0x60;
  switch (indexedRegOffs + 0x80)
  {
    case SDMA_CCRi:
      value = sdma->chIndexedRegs[channelIndex].ccr;
      found = TRUE;
      break;
    case SDMA_CSRi:
      value = sdma->chIndexedRegs[channelIndex].csr;
      found = TRUE;
      break;
    case SDMA_CSDPi:
      found = TRUE;
      value = sdma->chIndexedRegs[channelIndex].csdp;
      break;
    case SDMA_CLNK_CTRLi:
    case SDMA_CICRi:
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
  
  if (found)
  {
    DEBUG(VP_OMAP_35XX_SDMA, "%s: load from address %#.8x reg %#x value %#.8x" EOL, dev->deviceName,
        virtAddr, regOffs, value);
    return value;
  }
  
  DIE_NOW(NULL, "SDMA load from unknown register");
  
  return 0;
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
      sdma->irqEnableL0 = value;
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
      sdma->irqStatusL0 &= ~value;
      found = TRUE;
      break;
    case SDMA_IRQSTATUS_L1:
      sdma->irqStatusL1 &= ~value;
      found = TRUE;
      break;
    case SDMA_IRQSTATUS_L2:
      sdma->irqStatusL2 &= ~value;
      found = TRUE;
      break;
    case SDMA_IRQSTATUS_L3:
      sdma->irqStatusL3 &= ~value;
      found = TRUE;
      break;
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
#if !defined(CONFIG_NO_MMC) && !defined(CONFIG_HW_PASSTHROUGH)
  bool dispatched = FALSE;
#endif
  switch (indexedRegOffs+0x80)
  {
    case SDMA_CCRi:
    {
      if ((value & SDMA_CCRi_ENABLE) == SDMA_CCRi_ENABLE)
      {
        DEBUG(VP_OMAP_35XX_SDMA, "%s: Enabling channel %x" EOL, dev->deviceName, channelIndex);
#if !defined(CONFIG_NO_MMC) && !defined(CONFIG_HW_PASSTHROUGH)
        /* It would be cleaner to check the SDMA_SEL_SRC_DST_SYNC bit and use it to decide
           if we should decide based on source or address. However, MMC transfers with
           kernel 2.6.28-omap1 seem to sometimes synchronize on destination even if 
           it's a mmc read */
        switch (sdma->chIndexedRegs[channelIndex].cssa)
        {
          case SD_MMC1 + MMCHS_DATA:
            mmcDoDmaXfer(context, 0, 0);
            dispatched = TRUE;
            break;
        }
       
        if (!dispatched)
        {
          switch (sdma->chIndexedRegs[channelIndex].cdsa)
          {
            case SD_MMC1 + MMCHS_DATA:
              mmcDoDmaXfer(context, 0, 0);
              break;
            default:
              DIE_NOW(NULL, "DMA channel enabled for unknown source / destination");
          }
        }
#endif
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

void sdmaThrowInterrupt(GCONTXT *context, u32int dmaChannel)
{
  struct Sdma *sdma = context->vm.sdma;

  if ((1 << dmaChannel) & sdma->irqEnableL0)
  {
    sdma->irqStatusL0 = (1 << dmaChannel);
    throwInterrupt(context, SDMA_IRQ_0);
    DEBUG(VP_OMAP_35XX_SDMA, "Throwing interrupt for irqStatusL0\n");
  }

  if ((1 << dmaChannel) & sdma->irqEnableL1)
  {
    sdma->irqStatusL1 = (1 << dmaChannel);
    throwInterrupt(context, SDMA_IRQ_1);
    DEBUG(VP_OMAP_35XX_SDMA, "Throwing interrupt for irqStatusL1\n");
  }

  if ((1 << dmaChannel) & sdma->irqEnableL2)
  {
    sdma->irqStatusL2 = (1 << dmaChannel);
    throwInterrupt(context, SDMA_IRQ_2);
    DEBUG(VP_OMAP_35XX_SDMA, "Throwing interrupt for irqStatusL2\n");
  }

  if ((1 << dmaChannel) & sdma->irqEnableL3)
  {
    sdma->irqStatusL3 = (1 << dmaChannel);
    throwInterrupt(context, SDMA_IRQ_3);
    DEBUG(VP_OMAP_35XX_SDMA, "Throwing interrupt for irqStatusL3\n");
  }
}
