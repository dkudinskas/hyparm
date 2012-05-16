#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sdrc.h"


struct Sdrc *sdrc;

void initSdrc()
{
  /**
   * initialization of SDRC
   */
  sdrc = (struct Sdrc *)calloc(1, sizeof(struct Sdrc));
  if (sdrc == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate SDRC.");
  }

  DEBUG(VP_OMAP_35XX_SDRC, "initSdrc @ %p" EOL, sdrc);

  sdrc->sysConfig =   0x10;
  sdrc->sysStatus =   0x1;
  sdrc->csCfg =       0x4;
  sdrc->sharing =     0;
  sdrc->errAddr =     0;
  sdrc->errType =     0x4;
  sdrc->dllaCtrl =    0;
  sdrc->dllaStatus =  0;
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


u32int loadSdrc(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "SDRC: invalid access size.");
  }

  u32int regOffset = phyAddr - Q1_L3_SDRC;
  u32int value = 0;

  switch (regOffset)
  {
    default:
    {
      printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
          virtAddr, (u32int)size);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }

  DEBUG(VP_OMAP_35XX_SDRC, "%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %#x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  return value;
}


void storeSdrc(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_SDRC, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  u32int regOffset = phyAddr - Q1_L3_SDRC;

  switch (regOffset)
  {
    default:
    {
      printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
          virtAddr, (u32int)size);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}
