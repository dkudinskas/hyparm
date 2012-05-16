#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "vm/omap35xx/mmc.h"

struct Mmc *mmc[3];

void initMmc(u32int mmcNumber)
{
  u32int id = mmcNumber-1;
  mmc[id] = (struct Mmc *)calloc(1, sizeof(struct Mmc));
  if (!mmc[id])
  {
    DIE_NOW(NULL, "Failed to allocate Mmc.");
  }

  memset(mmc[id], 0, sizeof(struct Mmc));

  DEBUG(VP_OMAP_35XX_MMC, "initMmc @ %p" EOL, mmc[id]);

  mmc[id]->mmcSysconfig = 0x15;
  mmc[id]->mmcSysstatus = 0x1;
  mmc[id]->mmcCsre      = 0;
  mmc[id]->mmcSystest   = 0;
  mmc[id]->mmcCon       = 0;
  mmc[id]->mmcPwcnt     = 0;
  mmc[id]->mmcBlk       = 0;
  mmc[id]->mmcArg       = 0;
  mmc[id]->mmcCmd       = 0;
  mmc[id]->mmcRsp10     = 0;
  mmc[id]->mmcRsp32     = 0;
  mmc[id]->mmcRsp54     = 0;
  mmc[id]->mmcRsp76     = 0;
  mmc[id]->mmcData      = 0;
  mmc[id]->mmcPstate    = 0;
  mmc[id]->mmcHctl      = 0;
  mmc[id]->mmcSysctl    = 0;
  mmc[id]->mmcStat      = 0;
  mmc[id]->mmcIe        = 0;
  mmc[id]->mmcIse       = 0;
  mmc[id]->mmcAc12      = 0;
  mmc[id]->mmcCapa      = 0;
  mmc[id]->mmcCurCapa   = 0;
  mmc[id]->mmcRev       = 0;
}

u32int getMmcId(u32int phyAddr)
{
  u32int id = 0;
  switch (phyAddr & 0xFFFFF000)
  {
    case SD_MMC1:
    {
      id = 0;
      break;
    }
    case SD_MMC2:
    {
      id = 1;
      break;
    }
    case SD_MMC3:
    {
      id = 2;
      break;
    }
    default:
      DIE_NOW(NULL, "Invalid MMC device.");
  }
  return id;
}

u32int loadMmc(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val = 0;

  u32int id = getMmcId(phyAddr);

  switch (phyAddr & 0xFFF) 
  {
    case MMCHS_SYSCONFIG:
    {
      val = mmc[id]->mmcSysconfig;
      break;
    }
    case MMCHS_SYSSTATUS:
    {
      val = mmc[id]->mmcSysstatus;
      break;
    }
    case MMCHS_HCTL: // 0x128
    {
      val = mmc[id]->mmcHctl;
      break;
    }
    case MMCHS_CAPA:
    {
      val = mmc[id]->mmcCapa;
      break;
    }
    case MMCHS_CON: // 0x02C
    {
      val = mmc[id]->mmcCon;
      break;
    }
    case MMCHS_SYSCTL: // 0x12c
    {
      val = mmc[id]->mmcSysctl;
      break;
    }
    case MMCHS_IE: // 0x134
    {
      val = mmc[id]->mmcIe;
      break;
    }
    case MMCHS_ISE: // 0x138
    {
      val = mmc[id]->mmcIse;
      break;
    }
    case MMCHS_STAT:
    {
      val = mmc[id]->mmcStat;
      break;
    }
    case MMCHS_BLK:
    {
      val = mmc[id]->mmcBlk;
      break;
    }
    case MMCHS_ARG:
    {
      val = mmc[id]->mmcArg;
      break;
    }
    case MMCHS_RSP10:
    {
      val = mmc[id]->mmcRsp10;
      break;
    }
    case MMCHS_RSP32:
    {
      val = mmc[id]->mmcRsp32;
      break;
    }
    case MMCHS_RSP54:
    {
      val = mmc[id]->mmcRsp54;
      break;
    }
    case MMCHS_RSP76:
    {
      val = mmc[id]->mmcRsp76;
      break;
    }
    default:
    {
      printf("WARNING: MMC read from register: 0x%x\n", phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }

  DEBUG(VP_OMAP_35XX_MMC, "MMC%d read 0x%x from 0x%x\n", id, val, phyAddr & 0xFFF);

  return val;
}

void storeMmc(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_MMC, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);
  u32int id = getMmcId(phyAddr);

  switch (phyAddr & 0xFFF)
  {
    case MMCHS_SYSCONFIG:
    {
      if (mmc[id]->mmcSysconfig != value)
      {
        printf("%s: unimplemented store to reg sysConfig" EOL, __func__);
      }
      break;
    }
    default:
    {
      printf("WARNING: MMC read from register: 0x%x\n", phyAddr);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}
