#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"
#include "common/helpers.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/mmc.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/sdma.h"

#include "io/mmc.h"

#include "memoryManager/pageTable.h"
#include "memoryManager/mmu.h"

struct Mmc *mmc[3];
extern struct mmc *mmcDevice;

u32int getMmcId(u32int address);
int getDmaChannelId(struct Sdma *sdma, u32int mmcAddress, bool read);
void mmcContinueDmaTransfer(GCONTXT *context, u32int id, u32int dmaChannel, bool read);
void mmcStartDmaTransfer(GCONTXT *context, u32int phyAddr, u32int id, bool read);
void mmcThrowInterrupt(GCONTXT *context, u32int mmcId);

u32int getMmcId(u32int address)
{
  switch (address & 0xFFFFF000)
  {
    case SD_MMC1:
      return 0;
    case SD_MMC2:
      return 1;
    case SD_MMC3:
      return 2;
    default:
      DIE_NOW(NULL, "Invalid MMC device.");
  }
}

int getDmaChannelId(struct Sdma *sdma, u32int mmcAddress, bool read)
{
  int id;
  
  if (read) 
  {
    for (id = 0; id < SDMA_CHANNEL_COUNT; id++)
    {
      if (sdma->chIndexedRegs[id].cssa == mmcAddress + MMCHS_DATA)
      {
        return id;
      }
    }
  }
  else 
  {
    for (id = 0; id < SDMA_CHANNEL_COUNT; id++)
    {
      if (sdma->chIndexedRegs[id].cdsa == mmcAddress + MMCHS_DATA)
      {
        return id;
      }
    }
  }
  
  return -1;
}

void initVirtMmc(virtualMachine *vm, u32int mmcNumber) 
{
  mmc[mmcNumber -1] = (struct Mmc *)calloc(1, sizeof(struct Mmc));
  if (!mmc[mmcNumber -1])
  {
    DIE_NOW(NULL, "Failed to allocate Mmc.");
  }

  memset(mmc[mmcNumber -1], 0, sizeof(struct Mmc));

  printf ("mmc%d initialised\n", mmcNumber);

#ifdef CONFIG_MMC_GUEST_ACCESS
  if (mmcNumber == 1)
  {
    u32int err = 0;
    if ((err = mmcMainInit()) != 0)
    {
      printf("Failed to initialize MMC host controller.\n");
      printf("Continuing, but no MMC card will be connected to the guest OS\n");
    }
    else
    {
      mmc[mmcNumber -1]->cardPresent = TRUE;
      printf("Giving guest access to MMC0\n");
    }
  }
#endif
}

u32int loadMmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val = 0;
  int id = getMmcId(phyAddr);

  switch (phyAddr & 0xFFF) 
  {
    case MMCHS_SYSCONFIG:
      val = mmc[id]->mmcSysconfig;
      break;
    case MMCHS_SYSSTATUS:
      val = mmc[id]->mmcSysstatus;
      break;
    case MMCHS_HCTL: // 0x128
      val = mmc[id]->mmcHctl;
      break;
    case MMCHS_CAPA:
      val = mmc[id]->mmcCapa;
      break;
    case MMCHS_CON: // 0x02C
      val = mmc[id]->mmcCon;
      break;
    case MMCHS_SYSCTL: // 0x12c
      val = mmc[id]->mmcSysctl;
      break;
    case MMCHS_IE: // 0x134
      val = mmc[id]->mmcIe;
      break;
    case MMCHS_ISE: // 0x138
      val = mmc[id]->mmcIse;
      break;
    case MMCHS_STAT:
      val = mmc[id]->mmcStat;
      break;
    case MMCHS_BLK:
      val = mmc[id]->mmcBlk;
      break;
    case MMCHS_ARG:
      val = mmc[id]->mmcArg;
      break;
    case MMCHS_RSP10:
      val = mmc[id]->mmcRsp10;
      break;
    case MMCHS_RSP32:
      val = mmc[id]->mmcRsp32;
      break;
    case MMCHS_RSP54:
      val = mmc[id]->mmcRsp54;
      break;
    case MMCHS_RSP76:
      val = mmc[id]->mmcRsp76;
      break;
    default:
      printf("WARNING: MMC read from register: 0x%x\n", phyAddr);
      DIE_NOW(NULL, "MMC read from unimplemented register");
  }

  DEBUG(VP_OMAP_35XX_MMC, "MMC%d read 0x%x from 0x%x\n", id, val, phyAddr & 0xFFF);

  return val;
}

void mmcThrowInterrupt(GCONTXT *context, u32int mmcId) 
{
  switch (mmcId)
  {
    case 0:
      throwInterrupt(context, MMC1_IRQ);
      break;
    case 1:
      throwInterrupt(context, MMC2_IRQ);
      break;
    case 2:
      throwInterrupt(context, MMC3_IRQ);
      break;
  }
}

void mmcStartDmaTransfer(GCONTXT *context, u32int phyAddr, u32int id, bool read)
{
#ifdef CONFIG_MMC_GUEST_ACCESS
  int dmaChannel;
  u32int noOfBlocksMmc;
  u32int noOfBlocksDma;
  u32int noOfTransferredBlocks;

  dmaChannel = getDmaChannelId(context->vm.sdma, phyAddr & (~0xFFF), read);
  DEBUG(VP_OMAP_35XX_MMC, "MMC dma channel: %d\n", dmaChannel);
  if (dmaChannel == -1)
  {
    DIE_NOW(NULL, "MMC Error: Can't find DMA channel\n");
  }

  noOfBlocksMmc = mmc[id]->mmcBlk >> 16;
  noOfBlocksDma = context->vm.sdma->chIndexedRegs[dmaChannel].cfn & 0xFFFF;
  DEBUG(VP_OMAP_35XX_MMC, "No of blocks (MMC): %d\n", noOfBlocksMmc);
  DEBUG(VP_OMAP_35XX_MMC, "No of blocks (DMA): %d\n", noOfBlocksDma);

  noOfTransferredBlocks = min(noOfBlocksMmc, noOfBlocksDma);
  
  bool replacedTTBR0 = FALSE;
  if (mmuGetTTBR0() != context->hypervisorPageTable)
  {
    mmuSetTTBR0(context->hypervisorPageTable, 0x1FF);
    replacedTTBR0 = TRUE;
  }

  if (read)
  {
    mmcDevice->blockDev.blockRead(mmcDevice->blockDev.devID,
                                  mmc[id]->mmcArg,
                                  noOfTransferredBlocks,
                                  (u32int *)context->vm.sdma->chIndexedRegs[dmaChannel].cdsa);//TODO //findVAforPA(sdma->chIndexedRegs[dmaChannel].cdsa));
  }
  else
  {
    mmcDevice->blockDev.blockWrite(mmcDevice->blockDev.devID,
                                   mmc[id]->mmcArg,
                                   noOfTransferredBlocks,
                                   (u32int *)context->vm.sdma->chIndexedRegs[dmaChannel].cssa);// TODO /findVAforPA(sdma->chIndexedRegs[dmaChannel].cssa));
  }
  
  if (replacedTTBR0) {
    mmuSetTTBR0(context->pageTables->shadowActive, context->pageTables->contextID);
  }
  
  context->vm.sdma->chIndexedRegs[dmaChannel].ccfn = noOfTransferredBlocks;
  context->vm.sdma->chIndexedRegs[dmaChannel].cicr = 0;

  DEBUG(VP_OMAP_35XX_MMC, "\nMMC Throwing interrupt(s)\n\n");
  
  if (noOfBlocksMmc > noOfBlocksDma) 
  {
    context->vm.sdma->chIndexedRegs[dmaChannel].csr = 1;
    mmc[id]->mmcStat = 0;
  }
  else
  {
    context->vm.sdma->chIndexedRegs[dmaChannel].csr = (1 << 5);
              
    mmc[id]->mmcRsp10 = 0;
    mmc[id]->mmcStat = MMC_CC | MMC_TC;
    mmcThrowInterrupt(context, id);
  }

  sdmaThrowInterrupt(context, dmaChannel);
#endif
}

void storeMmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  int dmaChannel = 0;
  u8int *targetAddress;
  u32int csize;
  bool   throwMmcInterrupt = FALSE;
  bool   throwSdmaInterrupt = FALSE;

  int id = getMmcId(phyAddr);

  DEBUG(VP_OMAP_35XX_MMC, "MMC%d store 0x%x to 0x%x\n", id, value, phyAddr & 0xFFF);

  switch (phyAddr & 0xFFF)
  {
    case MMCHS_SYSCONFIG:
      if (value & MMC_SOFTRESET)
      {
        mmc[id]->mmcSysstatus |= MMC_RESETDONE;
      }
      mmc[id]->mmcSysconfig = value & ~MMC_SOFTRESET;
      break;
    case MMCHS_HCTL: // 0x128
      mmc[id]->mmcHctl = value;
      break;
    case MMCHS_CAPA:  
      mmc[id]->mmcCapa = value;
      break;
    case MMCHS_SYSCTL: // 0x12c
      if (value & MMC_ICE)
        value |= MMC_ICS;
      if (value & MMC_SRC)
        value &= ~MMC_SRC;
      if (value & MMC_SRD)
        value &= ~MMC_SRD;
      mmc[id]->mmcSysctl = value;
      break;
    case MMCHS_CON: // 0x02C
      if (value & MMC_INIT)
      {
        DEBUG(VP_OMAP_35XX_MMC, "MMC init\n");

        mmc[id]->mmcStat = MMC_CC;
        throwMmcInterrupt = TRUE;
      }
      mmc[id]->mmcCon = value;
      break;
    case MMCHS_IE: // 0x134
      mmc[id]->mmcIe = value;
      break;
    case MMCHS_ISE: // 0x138
      mmc[id]->mmcIse = value;
      break;
    case MMCHS_CMD: // 0x10c
      if (id != 0)
      {
        DIE_NOW(NULL, "You can only execute commands on MMC0\n");
      }
      DEBUG(VP_OMAP_35XX_MMC, "MMC CMD: 0x%x, ARG: 0x%x\n", value, mmc[id]->mmcArg);

      if (MMC_CMD_INDEX(mmc[id]->mmcCmd) == 55)
      {
        switch (MMC_CMD_INDEX(value))
        {
          case 41:
            // OCR - Card not busy and can operate at any voltage
            mmc[id]->mmcRsp10 = (1 << 31) | (0xffff << 8);
            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            break;
          case 51:
            if (!(value & MMC_DE))
            {
              DIE_NOW(NULL, "Sorry, no support for PIO");
            }
            
            dmaChannel = getDmaChannelId(context->vm.sdma, phyAddr & (~0xFFF), MMC_READ);
            DEBUG(VP_OMAP_35XX_MMC, "MMC dma channel: %d\n", dmaChannel);
            if (dmaChannel == -1)
            {
              DIE_NOW(NULL, "MMC Error: Can't find DMA channel\n");
            }

            DEBUG(VP_OMAP_35XX_MMC, "hyp page table: %p\n", context->hypervisorPageTable);
            DEBUG(VP_OMAP_35XX_MMC, "shadow active: %p\n", context->pageTables->shadowActive);
            DEBUG(VP_OMAP_35XX_MMC, "TTBR0: %p\n", mmuGetTTBR0());
            
            bool replacedTTBR0 = FALSE;
            if (mmuGetTTBR0() != context->hypervisorPageTable)
            {
              mmuSetTTBR0(context->hypervisorPageTable, 0x1FF);
              replacedTTBR0 = TRUE;
            }

            targetAddress = (u8int *)context->vm.sdma->chIndexedRegs[dmaChannel].cdsa;// TODO findVAforPA(sdma->chIndexedRegs[dmaChannel].cdsa);
            DEBUG(VP_OMAP_35XX_MMC, "writing to pa: %p, va: %p\n", (u8int *)context->vm.sdma->chIndexedRegs[dmaChannel].cdsa, targetAddress);

            *(targetAddress++) = 00;
            *(targetAddress++) = 80;
            *(targetAddress++) = 31;
            *(targetAddress++) = 02;
            
            
            *(targetAddress++) = 00;
            *(targetAddress++) = 00;
            *(targetAddress++) = 00;
            *(targetAddress++) = 01;
            
            if (replacedTTBR0) {
              mmuSetTTBR0(context->pageTables->shadowActive, context->pageTables->contextID);
            }
            
            mmc[id]->mmcRsp10 = 0;            
            mmc[id]->mmcStat = MMC_CC | MMC_TC;
            throwMmcInterrupt = TRUE;
            
            context->vm.sdma->chIndexedRegs[dmaChannel].csr = (1 << 5) | (1 << 7);
            throwSdmaInterrupt = TRUE;
            
            break;
          default:
            DIE_NOW(NULL, "MMC unknown ACMD\n");
        }
      }
      else
      {
        switch (MMC_CMD_INDEX(value))
        {
          case 0:
            mmc[id]->mmcRsp10 = 0;
            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            break;
          case 2:
            DEBUG(VP_OMAP_35XX_MMC, "SEND CID\n");

            // TODO: Replace with own data
            mmc[id]->mmcRsp76 = 0x02544d53;
            mmc[id]->mmcRsp54 = 0x41303447;
            mmc[id]->mmcRsp32 = 0x06131ee4;
            mmc[id]->mmcRsp10 = 0x2100b100;

            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;

            break;
          case 3:
            DEBUG(VP_OMAP_35XX_MMC, "Send RCA\n");
            
            mmc[id]->mmcRsp10 = 1234 << 16;
            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            
            break;
          case 5:
            DEBUG(VP_OMAP_35XX_MMC, "MMC CMD5? This isn't an SDIO card.\n");
            mmc[id]->mmcStat = MMC_CTO | MMC_ERRI;
            throwMmcInterrupt = TRUE;
            break;
          case 7:
            DEBUG(VP_OMAP_35XX_MMC, "MMC CMD7. TODO: Select/deselect\n");
            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            break;
          case 8:
            DEBUG(VP_OMAP_35XX_MMC, "MMC CMD8\n");

            mmc[id]->mmcRsp10 = mmc[id]->mmcArg & 0xFFF;
            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            break;
          case 9:
            DEBUG(VP_OMAP_35XX_MMC, "Sending CSD\n");
            if (!mmc[0]->cardPresent)
            {
              DIE_NOW(NULL, "No MMC card connected to the guest OS, why is it reading this?\n");
            }
#ifdef CONFIG_MMC_GUEST_ACCESS
            csize = (mmcDevice->capacity / (512 * 1024)) -1;
            // TODO: Double check that all these properties make sense
            mmc[id]->mmcRsp76 = 0x400e0032;
            mmc[id]->mmcRsp54 = 0x5b590000 | ((csize >> 16) & 0x3F);
            mmc[id]->mmcRsp32 = ((csize & 0xFFFF) << 16) | 0x7f80;
            mmc[id]->mmcRsp10 = 0x0a400000;
#endif
            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            
            break;
          case 12:
            mmc[id]->mmcRsp10 = 0;

            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            
            break;
          case 13:
            DEBUG(VP_OMAP_35XX_MMC, "CMD13\n");
            
            mmc[id]->mmcRsp10 = (4 << 9) | (1 << 8);

            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            
            break;
          case 17:
          case 18:
            if (!(value & MMC_DE))
            {
              DIE_NOW(NULL, "Sorry, no support for PIO");
            }
            
            mmcStartDmaTransfer(context, phyAddr, id, MMC_READ);
          
            break;
          case 24:
          case 25:
            if (!(value & MMC_DE))
            {
              DIE_NOW(NULL, "Sorry, no support for PIO");
            }
            
            mmcStartDmaTransfer(context, phyAddr, id, MMC_WRITE);

            break;
          case 55:
            DEBUG(VP_OMAP_35XX_MMC, "CMD55, waiting for ACMD\n");
            mmc[id]->mmcRsp10 = MMC_APP_CMD;
            mmc[id]->mmcStat = MMC_CC;
            throwMmcInterrupt = TRUE;
            
            break;

          default:
            DIE_NOW(NULL, "Unimplemented MMC command\n");
        }
      }

      mmc[id]->mmcCmd = value;

      break;
    case MMCHS_STAT:
      mmc[id]->mmcStat &= ~value;
      break;
    case MMCHS_BLK:
      mmc[id]->mmcBlk = value;
      break;
    case MMCHS_ARG:
      mmc[id]->mmcArg = value;
      break;
    default:
      printf("WARNING: MMC default store of 0x%x at offset: 0x%x\n", value, phyAddr);
      DIE_NOW(NULL, "MMC wrote to unimplemented register\n");
  }
  
  if (throwMmcInterrupt)
  {
    mmcThrowInterrupt(context, id);
  }

  if (throwSdmaInterrupt)
  {
    sdmaThrowInterrupt(context, dmaChannel);
  }
}

void mmcContinueDmaTransfer(GCONTXT *context, u32int id, u32int dmaChannel, bool read) 
{
#ifdef CONFIG_MMC_GUEST_ACCESS
  u32int noOfBlocksMmc;
  u32int noOfBlocksDma;
 
  noOfBlocksMmc = mmc[id]->mmcBlk >> 16;

  if (context->vm.sdma->chIndexedRegs[dmaChannel].ccfn > 0 
      && context->vm.sdma->chIndexedRegs[dmaChannel].ccfn < noOfBlocksMmc)
  {
    // Continue transfer
    noOfBlocksDma = context->vm.sdma->chIndexedRegs[dmaChannel].cfn & 0xFFFF;
    
    bool replacedTTBR0 = FALSE;
    if (mmuGetTTBR0() != context->hypervisorPageTable)
    {
      mmuSetTTBR0(context->hypervisorPageTable, 0x1FF);
      replacedTTBR0 = TRUE;
    }
    if (read)
    {
      mmcDevice->blockDev.blockRead(mmcDevice->blockDev.devID,
                                  mmc[id]->mmcArg + context->vm.sdma->chIndexedRegs[dmaChannel].ccfn,
                                  noOfBlocksDma,
                                  (u32int *)context->vm.sdma->chIndexedRegs[dmaChannel].cdsa);// TODO findVAforPA(sdma->chIndexedRegs[dmaChannel].cdsa));
    }
    else
    {
      mmcDevice->blockDev.blockWrite(mmcDevice->blockDev.devID,
                                      mmc[id]->mmcArg + context->vm.sdma->chIndexedRegs[dmaChannel].ccfn,
                                      noOfBlocksDma,
                                      (u32int *)context->vm.sdma->chIndexedRegs[dmaChannel].cssa);// TODO findVAforPA(sdma->chIndexedRegs[dmaChannel].cssa));
    }
    
    if (replacedTTBR0) {
      mmuSetTTBR0(context->pageTables->shadowActive, context->pageTables->contextID);
    }
    
    context->vm.sdma->chIndexedRegs[dmaChannel].ccfn += noOfBlocksDma;
        
    DEBUG(VP_OMAP_35XX_MMC, "MMC:Progress: %d blocks\n", context->vm.sdma->chIndexedRegs[dmaChannel].ccfn);

    DEBUG(VP_OMAP_35XX_MMC, "MMC: Throw interrupt(s)\n");
    context->vm.sdma->chIndexedRegs[dmaChannel].cicr = 0;

    if (context->vm.sdma->chIndexedRegs[dmaChannel].ccfn < noOfBlocksMmc)
    {
      context->vm.sdma->chIndexedRegs[dmaChannel].csr = 1;
    }
    else
    {
      context->vm.sdma->chIndexedRegs[dmaChannel].csr = (1 << 5);

      mmc[id]->mmcRsp10 = 0;
      mmc[id]->mmcStat = MMC_CC | MMC_TC;
          
      mmcThrowInterrupt(context, id);
    }

    sdmaThrowInterrupt(context, dmaChannel);
  }
#endif
}

void mmcDoDmaXfer(GCONTXT *context, int id, int dmaChannel)
{
  DEBUG(VP_OMAP_35XX_MMC, "mmcDoDmaXfer\n");

  if (context->vm.sdma->chIndexedRegs[dmaChannel].cicr == 0)
  {
    return;
  }

  switch (MMC_CMD_INDEX(mmc[id]->mmcCmd))
  {
    case 18:
      mmcContinueDmaTransfer(context, id, dmaChannel, MMC_READ);
      break;
    case 25:
      mmcContinueDmaTransfer(context, id, dmaChannel, MMC_WRITE);
      break;
  }
}

