#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdrc.h"


/************************
 * REGISTER DEFINITIONS *
 ************************/

#define SDRC_REVISION           0x00
#define SDRC_SYSCONFIG          0x10
#define SDRC_SYSSTATUS          0x14
#define SDRC_CS_CFG             0x40
#define SDRC_SHARING            0x44
#define SDRC_ERR_ADDR           0x48
#define SDRC_ERR_TYPE           0x4C
#define SDRC_DLLA_CTRL          0x60
#define SDRC_DLLA_STATUS        0x64
#define SDRC_POWER_REG          0x70

#define SDRC_MCFG(i)            (0x80 + ((i)*0x30))
#define SDRC_MR(i)              (0x84 + ((i)*0x30))
#define SDRC_EMR2(i)            (0x8C + ((i)*0x30))
#define SDRC_ACTIM_CTRLA(i)     (0x9C + ((i)*0x28))
#define SDRC_ACTIM_CTRLB(i)     (0xA0 + ((i)*0x28))
#define SDRC_RFR_CTRL(i)        (0xA4 + ((i)*0x30))
#define SDRC_MANUAL(i)          (0xA8 + ((i)*0x30))


/**************************
 * STATIC REGISTER VALUES *
 **************************/

#define SDRC_REVISION_VALUE     0x00000040


void initSdrc(virtualMachine *vm)
{
  /**
   * initialization of SDRC
   */
  struct Sdrc *const sdrc = (struct Sdrc *)calloc(1, sizeof(struct Sdrc));
  if (sdrc == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate SDRC.");
  }
  vm->sdrc = sdrc;

  DEBUG(VP_OMAP_35XX_SDRC, "initSdrc @ %p" EOL, sdrc);

  sdrc->sysConfig =   0x10;
  sdrc->sysStatus =   0x1;
  sdrc->csCfg =       0x4;
  sdrc->sharing =     0;
  sdrc->errAddr =     0;
  sdrc->errType =     0x4;
  sdrc->dllaCtrl =    0xA;
  sdrc->dllaStatus =  0x4;
  sdrc->powerReg =    0x85;

  sdrc->mcfg0 =       0;
  sdrc->mcfg1 =       0;
  sdrc->mr0 =         0x24;
  sdrc->mr1 =         0x24;
  sdrc->emr20 =       0;
  sdrc->emr21 =       0;
  sdrc->actimCtrla0 = 0;
  sdrc->actimCtrla1 = 0;
  sdrc->actimCtrlb0 = 0;
  sdrc->actimCtrlb1 = 0;
  sdrc->rfrCtrl0 =    0;
  sdrc->rfrCtrl1 =    0;
  sdrc->manual0 =     0;
  sdrc->manual1 =     0;
}


u32int loadSdrc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  ASSERT(size == WORD, ERROR_BAD_ACCESS_SIZE);

  struct Sdrc *const sdrc = context->vm.sdrc;
  u32int regOffset = phyAddr - Q1_L3_SDRC;
  u32int value = 0;

  switch (regOffset)
  {
    case SDRC_SYSCONFIG:
    {
      value = sdrc->sysConfig;
      break;
    }
    case SDRC_SYSSTATUS:
    {
      value = sdrc->sysStatus;
      break;
    }
    case SDRC_CS_CFG:
    {
      value = sdrc->csCfg;
      break;
    }
    case SDRC_SHARING:
    {
      value = sdrc->sharing;
      break;
    }
    case SDRC_ERR_ADDR:
    {
      value = sdrc->errAddr;
      break;
    }
    case SDRC_ERR_TYPE:
    {
      value = sdrc->errType;
      break;
    }
    case SDRC_DLLA_CTRL:
    {
      value = sdrc->dllaCtrl;
      break;
    }
    case SDRC_DLLA_STATUS:
    {
      value = sdrc->dllaStatus;
      break;
    }
    case SDRC_POWER_REG:
    {
      value = sdrc->powerReg;
      break;
    }
    case SDRC_MCFG(0):
      {
        value = sdrc->mcfg0;
        break;
      }
    case SDRC_MCFG(1):
    {
      value = sdrc->mcfg1;
      break;
    }
    case SDRC_MR(0):
    {
      value = sdrc->mr0;
      break;
    }
    case SDRC_MR(1):
    {
      value = sdrc->mr1;
      break;
    }
    case SDRC_EMR2(0):
    {
      value = sdrc->emr20;
      break;
    }
    case SDRC_EMR2(1):
    {
      value = sdrc->emr21;
      break;
    }
    case SDRC_ACTIM_CTRLA(0):
    {
      value = sdrc->actimCtrla0;
      break;
    }
    case SDRC_ACTIM_CTRLA(1):
    {
      value = sdrc->actimCtrla1;
      break;
    }
    case SDRC_ACTIM_CTRLB(0):
    {
      value = sdrc->actimCtrlb0;
      break;
    }
    case SDRC_ACTIM_CTRLB(1):
    {
      value = sdrc->actimCtrlb1;
      break;
    }
    case SDRC_RFR_CTRL(0):
    {
      value = sdrc->rfrCtrl0;
      break;
    }
    case SDRC_RFR_CTRL(1):
    {
      value = sdrc->rfrCtrl1;
      break;
    }
    case SDRC_MANUAL(0):
      {
        value = sdrc->manual0;
        break;
      }
    case SDRC_MANUAL(1):
    {
      value = sdrc->manual1;
      break;
    }
    default:
    {
      printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
          virtAddr, (u32int)size);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }

  DEBUG(VP_OMAP_35XX_SDRC, "%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %#x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  return value;
}


void storeSdrc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_SDRC, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  struct Sdrc *const sdrc = context->vm.sdrc;
  u32int regOffset = phyAddr - Q1_L3_SDRC;

  switch (regOffset)
  {
    case SDRC_SYSCONFIG:
    {
      if (sdrc->sysConfig != value)
      {
        printf("%s: unimplemented store to reg sysConfig" EOL, __func__);
      }
      break;
    }
    case SDRC_DLLA_CTRL:
    {
      if (sdrc->dllaCtrl != value)
      {
        printf("%s: unimplemented store to reg dllaCtrl" EOL, __func__);
      }
      break;
    }
    case SDRC_POWER_REG:
    {
      if (sdrc->powerReg != value)
      {
        printf("%s: unimplemented store to reg powerReg" EOL, __func__);
      }
      break;
    }
    case SDRC_RFR_CTRL(0):
    {
      if (sdrc->rfrCtrl0 != value)
      {
        printf("%s: unimplemented store to reg rfrCtrl0" EOL, __func__);
      }
      break;
    }
    case SDRC_RFR_CTRL(1):
    {
      if (sdrc->rfrCtrl1 != value)
      {
        printf("%s: unimplemented store to reg rfrCtrl1" EOL, __func__);
      }
      break;
    }
    default:
    {
      printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x, value %x" EOL, dev->deviceName, phyAddr,
          virtAddr, (u32int)size, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}
