#ifndef __VM__OMAP_35XX__PM_H__
#define __VM__OMAP_35XX__PM_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct PmRt
{
  u32int pmErrorLog;
  u32int pmControl;
  u32int pmErrorClearSingle;
  u32int pmErrorClearMulti;

  u32int pmReqInfoPermission0;
  u32int pmReqInfoPermission1;

  u32int pmReadPermission0;
  u32int pmReadPermission1;

  u32int pmWritePermission0;
  u32int pmWritePermission1;

  u32int pmAddrMatch1;
};

struct PmGpmc
{
  u32int pmErrorLog;
  u32int pmControl;
  u32int pmErrorClearSingle;
  u32int pmErrorClearMulti;

  u32int pmReqInfoPermission0;
  u32int pmReqInfoPermission1;
  u32int pmReqInfoPermission2;
  u32int pmReqInfoPermission3;
  u32int pmReqInfoPermission4;
  u32int pmReqInfoPermission5;
  u32int pmReqInfoPermission6;
  u32int pmReqInfoPermission7;

  u32int pmReadPermission0;
  u32int pmReadPermission1;
  u32int pmReadPermission2;
  u32int pmReadPermission3;
  u32int pmReadPermission4;
  u32int pmReadPermission5;
  u32int pmReadPermission6;
  u32int pmReadPermission7;

  u32int pmWritePermission0;
  u32int pmWritePermission1;
  u32int pmWritePermission2;
  u32int pmWritePermission3;
  u32int pmWritePermission4;
  u32int pmWritePermission5;
  u32int pmWritePermission6;
  u32int pmWritePermission7;

  u32int pmAddrMatch1;
  u32int pmAddrMatch2;
  u32int pmAddrMatch3;
  u32int pmAddrMatch4;
  u32int pmAddrMatch5;
  u32int pmAddrMatch6;
  u32int pmAddrMatch7;
};

struct PmOcmRam
{
  u32int pmErrorLog;
  u32int pmControl;
  u32int pmErrorClearSingle;
  u32int pmErrorClearMulti;

  u32int pmReqInfoPermission0;
  u32int pmReqInfoPermission1;
  u32int pmReqInfoPermission2;
  u32int pmReqInfoPermission3;
  u32int pmReqInfoPermission4;
  u32int pmReqInfoPermission5;
  u32int pmReqInfoPermission6;
  u32int pmReqInfoPermission7;

  u32int pmReadPermission0;
  u32int pmReadPermission1;
  u32int pmReadPermission2;
  u32int pmReadPermission3;
  u32int pmReadPermission4;
  u32int pmReadPermission5;
  u32int pmReadPermission6;
  u32int pmReadPermission7;

  u32int pmWritePermission0;
  u32int pmWritePermission1;
  u32int pmWritePermission2;
  u32int pmWritePermission3;
  u32int pmWritePermission4;
  u32int pmWritePermission5;
  u32int pmWritePermission6;
  u32int pmWritePermission7;

  u32int pmAddrMatch1;
  u32int pmAddrMatch2;
  u32int pmAddrMatch3;
  u32int pmAddrMatch4;
  u32int pmAddrMatch5;
  u32int pmAddrMatch6;
  u32int pmAddrMatch7;
};

struct PmOcmRom
{
  u32int pmErrorLog;
  u32int pmControl;
  u32int pmErrorClearSingle;
  u32int pmErrorClearMulti;

  u32int pmReqInfoPermission0;
  u32int pmReqInfoPermission1;

  u32int pmReadPermission0;
  u32int pmReadPermission1;

  u32int pmWritePermission0;
  u32int pmWritePermission1;

  u32int pmAddrMatch1;
};

struct PmIva
{
  u32int pmErrorLog;
  u32int pmControl;
  u32int pmErrorClearSingle;
  u32int pmErrorClearMulti;

  u32int pmReqInfoPermission0;
  u32int pmReqInfoPermission1;
  u32int pmReqInfoPermission2;
  u32int pmReqInfoPermission3;

  u32int pmReadPermission0;
  u32int pmReadPermission1;
  u32int pmReadPermission2;
  u32int pmReadPermission3;

  u32int pmWritePermission0;
  u32int pmWritePermission1;
  u32int pmWritePermission2;
  u32int pmWritePermission3;

  u32int pmAddrMatch1;
  u32int pmAddrMatch2;
  u32int pmAddrMatch3;
};


void initProtectionMechanism(virtualMachine *vm) __cold__;
u32int loadProtectionMechanism(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeProtectionMechanism(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__PM_H__ */
