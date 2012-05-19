#ifndef __VM__OMAP_35XX__GPMC_H__
#define __VM__OMAP_35XX__GPMC_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct Gpmc
{
  u32int gpmcSysConfig;
  u32int gpmcSysStatus;
  u32int gpmcIrqStatus;
  u32int gpmcIrqEnable;
  u32int gpmcTimeoutControl;
  u32int gpmcErrAddress;
  u32int gpmcErrType;
  u32int gpmcConfig;
  u32int gpmcStatus;

  u32int gpmcConfig1_0;
  u32int gpmcConfig7_0;
  u32int gpmcConfig1_1;
  u32int gpmcConfig7_1;
  u32int gpmcConfig1_2;
  u32int gpmcConfig7_2;
  u32int gpmcConfig1_3;
  u32int gpmcConfig7_3;
  u32int gpmcConfig1_4;
  u32int gpmcConfig7_4;
  u32int gpmcConfig1_5;
  u32int gpmcConfig7_5;
  u32int gpmcConfig1_6;
  u32int gpmcConfig7_6;
  u32int gpmcConfig1_7;
  u32int gpmcConfig7_7;

  u32int gpmcPrefetchConfig1;
  u32int gpmcPrefetchConfig2;
  u32int gpmcPrefetchControl;
  u32int gpmcPrefetchStatus;
  u32int gpmcEccConfig;
  u32int gpmcEccControl;
  u32int gpmcEccSizeConfig;
};


void initGpmc(virtualMachine *vm) __cold__;
u32int loadGpmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeGpmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__GPMC_H__ */
