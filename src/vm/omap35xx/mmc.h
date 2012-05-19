#ifndef __VM__OMAP_35XX__MMC_H__
#define __VM__OMAP_35XX__MMC_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct Mmc 
{
  u32int mmcSysconfig;
  u32int mmcSysstatus;
  u32int mmcCsre;
  u32int mmcSystest;
  u32int mmcCon;
  u32int mmcPwcnt;
  u32int mmcBlk;
  u32int mmcArg;
  u32int mmcCmd;
  u32int mmcRsp10;
  u32int mmcRsp32;
  u32int mmcRsp54;
  u32int mmcRsp76;
  u32int mmcData;
  u32int mmcPstate;
  u32int mmcHctl;
  u32int mmcSysctl;
  u32int mmcStat;
  u32int mmcIe;
  u32int mmcIse;
  u32int mmcAc12;
  u32int mmcCapa;
  u32int mmcCurCapa;
  u32int mmcRev;
};


void initMmc(virtualMachine *vm, u32int mmcNumber) __cold__;
u32int loadMmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeMmc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__MMC_H__ */
