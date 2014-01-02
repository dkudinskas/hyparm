#ifndef __VM__OMAP_35XX__SDRC_H__
#define __VM__OMAP_35XX__SDRC_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct Sdrc
{
  u32int sysConfig;
  u32int sysStatus;
  u32int csCfg;
  u32int sharing;
  u32int errAddr;
  u32int errType;
  u32int dllaCtrl;
  u32int dllaStatus;
  u32int powerReg;

  u32int mcfg0;
  u32int mcfg1;
  u32int mr0;
  u32int mr1;
  u32int emr20;
  u32int emr21;
  u32int actimCtrla0;
  u32int actimCtrla1;
  u32int actimCtrlb0;
  u32int actimCtrlb1;
  u32int rfrCtrl0;
  u32int rfrCtrl1;
  u32int manual0;
  u32int manual1;
};


void initSdrc(virtualMachine *vm) __cold__;
u32int loadSdrc(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSdrc(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__SDRC_H__ */

