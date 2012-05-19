#include "common/debug.h"
#include "common/stdlib.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/mmc.h"


/************************
 * REGISTER DEFINITIONS *
 ************************/
#define MMCHS_SYSCONFIG 0x010
#define MMCHS_SYSSTATUS 0x014
#define MMCHS_CSRE      0x024
#define MMCHS_SYSTEST   0x028
#define MMCHS_CON       0x02C
#define MMCHS_PWCNT     0x030
#define MMCHS_BLK       0x104
#define MMCHS_ARG       0x108
#define MMCHS_CMD       0x10C
#define MMCHS_RSP10     0x110
#define MMCHS_RSP32     0x114
#define MMCHS_RSP54     0x118
#define MMCHS_RSP76     0x11C
#define MMCHS_DATA      0x120
#define MMCHS_PSTATE    0x124
#define MMCHS_HCTL      0x128
#define MMCHS_SYSCTL    0x12C
#define MMCHS_STAT      0x130
#define MMCHS_IE        0x134
#define MMCHS_ISE       0x138
#define MMCHS_AC12      0x13C
#define MMCHS_CAPA      0x140
#define MMCHS_CUR_CAPA  0x148
#define MMCHS_REV       0x1FC

#define MMC_SOFTRESET   0x2
#define MMC_RESETDONE   0x1

// MMCHS_SYSCTL
#define MMC_ICE         0x1
#define MMC_ICS         0x2
#define MMC_SRC         (1 << 25)
#define MMC_SRD         (1 << 26)

// MMCHS_CON
#define MMC_INIT        0x2

// MMCHS_STAT
#define MMC_CC          1
#define MMC_TC          (1 << 1)
#define MMC_BRR         (1 << 5)
#define MMC_ERRI        (1 << 15)
#define MMC_CTO         (1 << 16)

// MMC R1
#define MMC_APP_CMD     (1 << 5)

// MMC_CMD
#define MMC_DE          1

// helpers
#define MMC_CMD_INDEX(cmd) ((cmd >> 24) & 0x3f)

#define MMC_READ        TRUE
#define MMC_WRITE       FALSE


static u32int getIndexByAddress(u32int phyAddr);


void initMmc(virtualMachine *vm, u32int mmcNumber)
{
  u32int id = mmcNumber - 1;
  vm->mmc[id] = (struct Mmc *)calloc(1, sizeof(struct Mmc));
  if (!vm->mmc[id])
  {
    DIE_NOW(NULL, "Failed to allocate Mmc.");
  }

  DEBUG(VP_OMAP_35XX_MMC, "initMmc @ %p" EOL, &vm->mmc[id]);

  vm->mmc[id]->mmcSysconfig = 0x15;
  vm->mmc[id]->mmcSysstatus = 0x1;
  vm->mmc[id]->mmcCsre      = 0;
  vm->mmc[id]->mmcSystest   = 0;
  vm->mmc[id]->mmcCon       = 0;
  vm->mmc[id]->mmcPwcnt     = 0;
  vm->mmc[id]->mmcBlk       = 0;
  vm->mmc[id]->mmcArg       = 0;
  vm->mmc[id]->mmcCmd       = 0;
  vm->mmc[id]->mmcRsp10     = 0;
  vm->mmc[id]->mmcRsp32     = 0;
  vm->mmc[id]->mmcRsp54     = 0;
  vm->mmc[id]->mmcRsp76     = 0;
  vm->mmc[id]->mmcData      = 0;
  vm->mmc[id]->mmcPstate    = 0;
  vm->mmc[id]->mmcHctl      = 0;
  vm->mmc[id]->mmcSysctl    = 0;
  vm->mmc[id]->mmcStat      = 0;
  vm->mmc[id]->mmcIe        = 0;
  vm->mmc[id]->mmcIse       = 0;
  vm->mmc[id]->mmcAc12      = 0;
  vm->mmc[id]->mmcCapa      = 0;
  vm->mmc[id]->mmcCurCapa   = 0;
  vm->mmc[id]->mmcRev       = 0;
}

u32int getIndexByAddress(u32int phyAddr)
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

u32int loadMmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val = 0;

  u32int id = getIndexByAddress(phyAddr);
  struct Mmc *const mmc = context->vm.mmc[id];

  switch (phyAddr & 0xFFF) 
  {
    case MMCHS_SYSCONFIG:
    {
      val = mmc->mmcSysconfig;
      break;
    }
    case MMCHS_SYSSTATUS:
    {
      val = mmc->mmcSysstatus;
      break;
    }
    case MMCHS_HCTL: // 0x128
    {
      val = mmc->mmcHctl;
      break;
    }
    case MMCHS_CAPA:
    {
      val = mmc->mmcCapa;
      break;
    }
    case MMCHS_CON: // 0x02C
    {
      val = mmc->mmcCon;
      break;
    }
    case MMCHS_SYSCTL: // 0x12c
    {
      val = mmc->mmcSysctl;
      break;
    }
    case MMCHS_IE: // 0x134
    {
      val = mmc->mmcIe;
      break;
    }
    case MMCHS_ISE: // 0x138
    {
      val = mmc->mmcIse;
      break;
    }
    case MMCHS_STAT:
    {
      val = mmc->mmcStat;
      break;
    }
    case MMCHS_BLK:
    {
      val = mmc->mmcBlk;
      break;
    }
    case MMCHS_ARG:
    {
      val = mmc->mmcArg;
      break;
    }
    case MMCHS_RSP10:
    {
      val = mmc->mmcRsp10;
      break;
    }
    case MMCHS_RSP32:
    {
      val = mmc->mmcRsp32;
      break;
    }
    case MMCHS_RSP54:
    {
      val = mmc->mmcRsp54;
      break;
    }
    case MMCHS_RSP76:
    {
      val = mmc->mmcRsp76;
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

void storeMmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_MMC, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  u32int id = getIndexByAddress(phyAddr);
  struct Mmc *const mmc = context->vm.mmc[id];

  switch (phyAddr & 0xFFF)
  {
    case MMCHS_SYSCONFIG:
    {
      if (mmc->mmcSysconfig != value)
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
