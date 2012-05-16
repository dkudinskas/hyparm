#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/pm.h"


struct PmRt     *pmrt;
struct PmGpmc   *pmgpmc;
struct PmOcmRam *pmocmram;
struct PmOcmRom *pmocmrom;
struct PmIva    *pmiva;


void initProtectionMechanism()
{
  /**
   * initialization of PM_RT
   */
  pmrt = (struct PmRt *)calloc(1, sizeof(struct PmRt));
  if (pmrt == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate PM_RT.");
  }

  DEBUG(VP_OMAP_35XX_PM, "initProtectionMechanism: @ %p" EOL, pmrt);

  pmrt->pmControl            = 0x03000000;

  pmrt->pmReqInfoPermission0 = 0xFFFF;

  pmrt->pmReadPermission0    = 0x1406;
  pmrt->pmReadPermission1    = 0x1406;

  pmrt->pmWritePermission0   = 0x1406;
  pmrt->pmWritePermission1   = 0x1406;

  pmrt->pmAddrMatch1         = 0x10230;

  /**
   * initialization of PM_GPMC
   */
  pmgpmc = (struct PmGpmc *)calloc(1, sizeof(struct PmGpmc));
  if (pmgpmc == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate PM_GPMC.");
  }

  DEBUG(VP_OMAP_35XX_PM, "initProtectionMechanism: @ %p" EOL, pmgpmc);

  pmgpmc->pmControl            = 0x03000000;

  pmgpmc->pmReqInfoPermission3 = 0xFFFF;
  pmgpmc->pmReqInfoPermission4 = 0xFFFF;
  pmgpmc->pmReqInfoPermission5 = 0xFFFF;
  pmgpmc->pmReqInfoPermission6 = 0xFFFF;
  pmgpmc->pmReqInfoPermission7 = 0xFFFF;

  pmgpmc->pmReadPermission0    = 0xD63E;
  pmgpmc->pmReadPermission1    = 0xD63E;
  pmgpmc->pmReadPermission2    = 0xD63E;
  pmgpmc->pmReadPermission3    = 0xD63E;
  pmgpmc->pmReadPermission4    = 0xD63E;
  pmgpmc->pmReadPermission5    = 0xD63E;
  pmgpmc->pmReadPermission6    = 0xD63E;
  pmgpmc->pmReadPermission7    = 0xD63E;

  pmgpmc->pmWritePermission0   = 0xD63E;
  pmgpmc->pmWritePermission1   = 0xD63E;
  pmgpmc->pmWritePermission2   = 0xD63E;
  pmgpmc->pmWritePermission3   = 0xD63E;
  pmgpmc->pmWritePermission4   = 0xD63E;
  pmgpmc->pmWritePermission5   = 0xD63E;
  pmgpmc->pmWritePermission6   = 0xD63E;
  pmgpmc->pmWritePermission7   = 0xD63E;

  pmgpmc->pmAddrMatch1         = 0x118;

  /**
   * initialization of PM_OCM_RAM
   */
  pmocmram = (struct PmOcmRam *)calloc(1, sizeof(struct PmOcmRam));
  if (pmocmram == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate PM_OCM_RAM.");
  }

  DEBUG(VP_OMAP_35XX_PM, "initProtectionMechanism: @ %p" EOL, pmocmram);

  pmocmram->pmControl            = 0x03000000;

  pmocmram->pmReqInfoPermission1 = 0xFFFF;
  pmocmram->pmReqInfoPermission2 = 0xFFFF;
  pmocmram->pmReqInfoPermission3 = 0xFFFF;
  pmocmram->pmReqInfoPermission4 = 0xFFFF;
  pmocmram->pmReqInfoPermission5 = 0xFFFF;
  pmocmram->pmReqInfoPermission6 = 0xFFFF;
  pmocmram->pmReqInfoPermission7 = 0xFFFF;

  pmocmram->pmReadPermission0    = 0xDF3E;
  pmocmram->pmReadPermission1    = 0xDF3E;
  pmocmram->pmReadPermission2    = 0xDF3E;
  pmocmram->pmReadPermission3    = 0xDF3E;
  pmocmram->pmReadPermission4    = 0xDF3E;
  pmocmram->pmReadPermission5    = 0xDF3E;
  pmocmram->pmReadPermission6    = 0xDF3E;
  pmocmram->pmReadPermission7    = 0xDF3E;

  pmocmram->pmWritePermission0   = 0xDF3E;
  pmocmram->pmWritePermission1   = 0xDF3E;
  pmocmram->pmWritePermission2   = 0xDF3E;
  pmocmram->pmWritePermission3   = 0xDF3E;
  pmocmram->pmWritePermission4   = 0xDF3E;
  pmocmram->pmWritePermission5   = 0xDF3E;
  pmocmram->pmWritePermission6   = 0xDF3E;
  pmocmram->pmWritePermission7   = 0xDF3E;

  pmocmram->pmAddrMatch2         = 0xF810;

  /**
   * initialization of PM_OCM_ROM
   */
  pmocmrom = (struct PmOcmRom *)calloc(1, sizeof(struct PmOcmRom));
  if (pmocmrom == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate PM_RT.");
  }

  DEBUG(VP_OMAP_35XX_PM, "initProtectionMechanism: @ %p" EOL, pmocmrom);

  pmocmrom->pmControl            = 0x03000000;

  pmocmrom->pmReqInfoPermission1 = 0xFFFF;

  pmocmrom->pmReadPermission0    = 0x1002;
  pmocmrom->pmReadPermission1    = 0x1002;

  pmocmrom->pmAddrMatch1         = 0x14028;

  /**
   * initialization of PM_IVA
   */
  pmiva = (struct PmIva *)calloc(1, sizeof(struct PmIva));
  if (pmiva == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate PM_IVA.");
  }

  DEBUG(VP_OMAP_35XX_PM, "initProtectionMechanism: @ %p" EOL, pmiva);

  pmiva->pmControl            = 0x03000000;

  pmiva->pmReqInfoPermission1 = 0xFFFF;
  pmiva->pmReqInfoPermission2 = 0xFFFF;
  pmiva->pmReqInfoPermission3 = 0xFFFF;

  pmiva->pmReadPermission0    = 0x140E;
  pmiva->pmReadPermission1    = 0x140E;
  pmiva->pmReadPermission2    = 0x140E;
  pmiva->pmReadPermission3    = 0x140E;

  pmiva->pmWritePermission0   = 0x140E;
  pmiva->pmWritePermission1   = 0x140E;
  pmiva->pmWritePermission2   = 0x140E;
  pmiva->pmWritePermission3   = 0x140E;
}

/* top load function */
u32int loadProtectionMechanism(device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "ProtectionMechanism: invalid access size.");
  }

  u32int value = 0;
  u32int regOffset = 0;

  if (phyAddr >= PM_RT && phyAddr < PM_GPMC)
  {
    regOffset = phyAddr - PM_RT;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        value = pmrt->pmErrorLog;
        break;
      case PM_CONTROL:
        value = pmrt->pmControl;
        break;
      case PM_ERROR_CLEAR_SINGLE:
        value = pmrt->pmErrorClearSingle;
        break;
      case PM_ERROR_CLEAR_MULTI:
        value = pmrt->pmErrorClearMulti;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        value = pmrt->pmReqInfoPermission0;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        value = pmrt->pmReqInfoPermission1;
        break;
      case PM_READ_PERMISSION_0:
        value = pmrt->pmReadPermission0;
        break;
      case PM_READ_PERMISSION_1:
        value = pmrt->pmReadPermission1;
        break;
      case PM_WRITE_PERMISSION_0:
        value = pmrt->pmWritePermission0;
        break;
      case PM_WRITE_PERMISSION_1:
        value = pmrt->pmWritePermission1;
        break;
      case PM_ADDR_MATCH_1:
        value = pmrt->pmAddrMatch1;
        break;
      default:
        printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_GPMC && phyAddr < PM_OCM_RAM)
  {
    regOffset = phyAddr - PM_GPMC;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        value = pmgpmc->pmErrorLog;
        break;
      case PM_CONTROL:
        value = pmgpmc->pmControl;
        break;
      case PM_ERROR_CLEAR_SINGLE:
        value = pmgpmc->pmErrorClearSingle;
        break;
      case PM_ERROR_CLEAR_MULTI:
        value = pmgpmc->pmErrorClearMulti;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        value = pmgpmc->pmReqInfoPermission0;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        value = pmgpmc->pmReqInfoPermission1;
        break;
      case PM_REQ_INFO_PERMISSION_2:
        value = pmgpmc->pmReqInfoPermission2;
        break;
      case PM_REQ_INFO_PERMISSION_3:
        value = pmgpmc->pmReqInfoPermission3;
        break;
      case PM_REQ_INFO_PERMISSION_4:
        value = pmgpmc->pmReqInfoPermission4;
        break;
      case PM_REQ_INFO_PERMISSION_5:
        value = pmgpmc->pmReqInfoPermission5;
        break;
      case PM_REQ_INFO_PERMISSION_6:
        value = pmgpmc->pmReqInfoPermission6;
        break;
      case PM_REQ_INFO_PERMISSION_7:
        value = pmgpmc->pmReqInfoPermission7;
        break;
      case PM_READ_PERMISSION_0:
        value = pmgpmc->pmReadPermission0;
        break;
      case PM_READ_PERMISSION_1:
        value = pmgpmc->pmReadPermission1;
        break;
      case PM_READ_PERMISSION_2:
        value = pmgpmc->pmReadPermission2;
        break;
      case PM_READ_PERMISSION_3:
        value = pmgpmc->pmReadPermission3;
        break;
      case PM_READ_PERMISSION_4:
        value = pmgpmc->pmReadPermission4;
        break;
      case PM_READ_PERMISSION_5:
        value = pmgpmc->pmReadPermission5;
        break;
      case PM_READ_PERMISSION_6:
        value = pmgpmc->pmReadPermission6;
        break;
      case PM_READ_PERMISSION_7:
        value = pmgpmc->pmReadPermission7;
        break;
      case PM_WRITE_PERMISSION_0:
        value = pmgpmc->pmWritePermission0;
        break;
      case PM_WRITE_PERMISSION_1:
        value = pmgpmc->pmWritePermission1;
        break;
      case PM_WRITE_PERMISSION_2:
        value = pmgpmc->pmWritePermission2;
        break;
      case PM_WRITE_PERMISSION_3:
        value = pmgpmc->pmWritePermission3;
        break;
      case PM_WRITE_PERMISSION_4:
        value = pmgpmc->pmWritePermission4;
        break;
      case PM_WRITE_PERMISSION_5:
        value = pmgpmc->pmWritePermission5;
        break;
      case PM_WRITE_PERMISSION_6:
        value = pmgpmc->pmWritePermission6;
        break;
      case PM_WRITE_PERMISSION_7:
        value = pmgpmc->pmWritePermission7;
        break;
      case PM_ADDR_MATCH_1:
        value = pmgpmc->pmAddrMatch1;
        break;
      case PM_ADDR_MATCH_2:
        value = pmgpmc->pmAddrMatch2;
        break;
      case PM_ADDR_MATCH_3:
        value = pmgpmc->pmAddrMatch3;
        break;
      case PM_ADDR_MATCH_4:
        value = pmgpmc->pmAddrMatch4;
        break;
      case PM_ADDR_MATCH_5:
        value = pmgpmc->pmAddrMatch5;
        break;
      case PM_ADDR_MATCH_6:
        value = pmgpmc->pmAddrMatch6;
        break;
      case PM_ADDR_MATCH_7:
        value = pmgpmc->pmAddrMatch7;
        break;
      default:
        printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_OCM_RAM && phyAddr < PM_OCM_ROM)
  {
    regOffset = phyAddr - PM_OCM_RAM;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        value = pmocmram->pmErrorLog;
        break;
      case PM_CONTROL:
        value = pmocmram->pmControl;
        break;
      case PM_ERROR_CLEAR_SINGLE:
        value = pmocmram->pmErrorClearSingle;
        break;
      case PM_ERROR_CLEAR_MULTI:
        value = pmocmram->pmErrorClearMulti;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        value = pmocmram->pmReqInfoPermission0;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        value = pmocmram->pmReqInfoPermission1;
        break;
      case PM_REQ_INFO_PERMISSION_2:
        value = pmocmram->pmReqInfoPermission2;
        break;
      case PM_REQ_INFO_PERMISSION_3:
        value = pmocmram->pmReqInfoPermission3;
        break;
      case PM_REQ_INFO_PERMISSION_4:
        value = pmocmram->pmReqInfoPermission4;
        break;
      case PM_REQ_INFO_PERMISSION_5:
        value = pmocmram->pmReqInfoPermission5;
        break;
      case PM_REQ_INFO_PERMISSION_6:
        value = pmocmram->pmReqInfoPermission6;
        break;
      case PM_REQ_INFO_PERMISSION_7:
        value = pmocmram->pmReqInfoPermission7;
        break;
      case PM_READ_PERMISSION_0:
        value = pmocmram->pmReadPermission0;
        break;
      case PM_READ_PERMISSION_1:
        value = pmocmram->pmReadPermission1;
        break;
      case PM_READ_PERMISSION_2:
        value = pmocmram->pmReadPermission2;
        break;
      case PM_READ_PERMISSION_3:
        value = pmocmram->pmReadPermission3;
        break;
      case PM_READ_PERMISSION_4:
        value = pmocmram->pmReadPermission4;
        break;
      case PM_READ_PERMISSION_5:
        value = pmocmram->pmReadPermission5;
        break;
      case PM_READ_PERMISSION_6:
        value = pmocmram->pmReadPermission6;
        break;
      case PM_READ_PERMISSION_7:
        value = pmocmram->pmReadPermission7;
        break;
      case PM_WRITE_PERMISSION_0:
        value = pmocmram->pmWritePermission0;
        break;
      case PM_WRITE_PERMISSION_1:
        value = pmocmram->pmWritePermission1;
        break;
      case PM_WRITE_PERMISSION_2:
        value = pmocmram->pmWritePermission2;
        break;
      case PM_WRITE_PERMISSION_3:
        value = pmocmram->pmWritePermission3;
        break;
      case PM_WRITE_PERMISSION_4:
        value = pmocmram->pmWritePermission4;
        break;
      case PM_WRITE_PERMISSION_5:
        value = pmocmram->pmWritePermission5;
        break;
      case PM_WRITE_PERMISSION_6:
        value = pmocmram->pmWritePermission6;
        break;
      case PM_WRITE_PERMISSION_7:
        value = pmocmram->pmWritePermission7;
        break;
      case PM_ADDR_MATCH_1:
        value = pmocmram->pmAddrMatch1;
        break;
      case PM_ADDR_MATCH_2:
        value = pmocmram->pmAddrMatch2;
        break;
      case PM_ADDR_MATCH_3:
        value = pmocmram->pmAddrMatch3;
        break;
      case PM_ADDR_MATCH_4:
        value = pmocmram->pmAddrMatch4;
        break;
      case PM_ADDR_MATCH_5:
        value = pmocmram->pmAddrMatch5;
        break;
      case PM_ADDR_MATCH_6:
        value = pmocmram->pmAddrMatch6;
        break;
      case PM_ADDR_MATCH_7:
        value = pmocmram->pmAddrMatch7;
        break;
      default:
        printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_OCM_ROM && phyAddr < PM_MAD2D)
  {
    regOffset = phyAddr - PM_OCM_ROM;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        value = pmocmrom->pmErrorLog;
        break;
      case PM_CONTROL:
        value = pmocmrom->pmControl;
        break;
      case PM_ERROR_CLEAR_SINGLE:
        value = pmocmrom->pmErrorClearSingle;
        break;
      case PM_ERROR_CLEAR_MULTI:
        value = pmocmrom->pmErrorClearMulti;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        value = pmocmrom->pmReqInfoPermission0;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        value = pmocmrom->pmReqInfoPermission1;
        break;
      case PM_READ_PERMISSION_0:
        value = pmocmrom->pmReadPermission0;
        break;
      case PM_READ_PERMISSION_1:
        value = pmocmrom->pmReadPermission1;
        break;
      case PM_WRITE_PERMISSION_0:
        value = pmocmrom->pmWritePermission0;
        break;
      case PM_WRITE_PERMISSION_1:
        value = pmocmrom->pmWritePermission1;
        break;
      case PM_ADDR_MATCH_1:
        value = pmocmrom->pmAddrMatch1;
        break;
      default:
        printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_IVA2 && phyAddr < (Q1_L3_PM + Q1_L3_PM_SIZE))
  {
    regOffset = phyAddr - PM_IVA2;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        value = pmiva->pmErrorLog;
        break;
      case PM_CONTROL:
        value = pmiva->pmControl;
        break;
      case PM_ERROR_CLEAR_SINGLE:
        value = pmiva->pmErrorClearSingle;
        break;
      case PM_ERROR_CLEAR_MULTI:
        value = pmiva->pmErrorClearMulti;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        value = pmiva->pmReqInfoPermission0;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        value = pmiva->pmReqInfoPermission1;
        break;
      case PM_REQ_INFO_PERMISSION_2:
        value = pmiva->pmReqInfoPermission2;
        break;
      case PM_REQ_INFO_PERMISSION_3:
        value = pmiva->pmReqInfoPermission3;
        break;
      case PM_READ_PERMISSION_0:
        value = pmiva->pmReadPermission0;
        break;
      case PM_READ_PERMISSION_1:
        value = pmiva->pmReadPermission1;
        break;
      case PM_READ_PERMISSION_2:
        value = pmiva->pmReadPermission2;
        break;
      case PM_READ_PERMISSION_3:
        value = pmiva->pmReadPermission3;
        break;
      case PM_WRITE_PERMISSION_0:
        value = pmiva->pmWritePermission0;
        break;
      case PM_WRITE_PERMISSION_1:
        value = pmiva->pmWritePermission1;
        break;
      case PM_WRITE_PERMISSION_2:
        value = pmiva->pmWritePermission2;
        break;
      case PM_WRITE_PERMISSION_3:
        value = pmiva->pmWritePermission3;
        break;
      case PM_ADDR_MATCH_1:
        value = pmiva->pmAddrMatch1;
        break;
      case PM_ADDR_MATCH_2:
        value = pmiva->pmAddrMatch2;
        break;
      case PM_ADDR_MATCH_3:
        value = pmiva->pmAddrMatch3;
        break;
      default:
        printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else
  {
    DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }
  return value;
}

/* top store function */
void storeProtectionMechanism(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_PM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  u32int regOffset = 0;

  if (phyAddr >= PM_RT && phyAddr < PM_GPMC)
  {
    regOffset = phyAddr - PM_RT;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        pmrt->pmErrorLog = value;
        break;
      case PM_CONTROL:
        pmrt->pmControl = value;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        pmrt->pmReqInfoPermission0 = value;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        pmrt->pmReqInfoPermission1 = value;
        break;
      case PM_READ_PERMISSION_0:
        pmrt->pmReadPermission0 = value;
        break;
      case PM_READ_PERMISSION_1:
        pmrt->pmReadPermission1 = value;
        break;
      case PM_WRITE_PERMISSION_0:
        pmrt->pmWritePermission0 = value;
        break;
      case PM_WRITE_PERMISSION_1:
        pmrt->pmWritePermission1 = value;
        break;
      case PM_ADDR_MATCH_1:
        pmrt->pmAddrMatch1 = value;
        break;
      default:
        printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x, value %#.8x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size, value);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_GPMC && phyAddr < PM_OCM_RAM)
  {
    regOffset = phyAddr - PM_GPMC;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        pmgpmc->pmErrorLog = value;
        break;
      case PM_CONTROL:
        pmgpmc->pmControl = value;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        pmgpmc->pmReqInfoPermission0 = value;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        pmgpmc->pmReqInfoPermission1 = value;
        break;
      case PM_REQ_INFO_PERMISSION_2:
        pmgpmc->pmReqInfoPermission2 = value;
        break;
      case PM_REQ_INFO_PERMISSION_3:
        pmgpmc->pmReqInfoPermission3 = value;
        break;
      case PM_REQ_INFO_PERMISSION_4:
        pmgpmc->pmReqInfoPermission4 = value;
        break;
      case PM_REQ_INFO_PERMISSION_5:
        pmgpmc->pmReqInfoPermission5 = value;
        break;
      case PM_REQ_INFO_PERMISSION_6:
        pmgpmc->pmReqInfoPermission6 = value;
        break;
      case PM_REQ_INFO_PERMISSION_7:
        pmgpmc->pmReqInfoPermission7 = value;
        break;
      case PM_READ_PERMISSION_0:
        pmgpmc->pmReadPermission0 = value;
        break;
      case PM_READ_PERMISSION_1:
        pmgpmc->pmReadPermission1 = value;
        break;
      case PM_READ_PERMISSION_2:
        pmgpmc->pmReadPermission2 = value;
        break;
      case PM_READ_PERMISSION_3:
        pmgpmc->pmReadPermission3 = value;
        break;
      case PM_READ_PERMISSION_4:
        pmgpmc->pmReadPermission4 = value;
        break;
      case PM_READ_PERMISSION_5:
        pmgpmc->pmReadPermission5 = value;
        break;
      case PM_READ_PERMISSION_6:
        pmgpmc->pmReadPermission6 = value;
        break;
      case PM_READ_PERMISSION_7:
        pmgpmc->pmReadPermission7 = value;
        break;
      case PM_WRITE_PERMISSION_0:
        pmgpmc->pmWritePermission0 = value;
        break;
      case PM_WRITE_PERMISSION_1:
        pmgpmc->pmWritePermission1 = value;
        break;
      case PM_WRITE_PERMISSION_2:
        pmgpmc->pmWritePermission2 = value;
        break;
      case PM_WRITE_PERMISSION_3:
        pmgpmc->pmWritePermission3 = value;
        break;
      case PM_WRITE_PERMISSION_4:
        pmgpmc->pmWritePermission4 = value;
        break;
      case PM_WRITE_PERMISSION_5:
        pmgpmc->pmWritePermission5 = value;
        break;
      case PM_WRITE_PERMISSION_6:
        pmgpmc->pmWritePermission6 = value;
        break;
      case PM_WRITE_PERMISSION_7:
        pmgpmc->pmWritePermission7 = value;
        break;
      case PM_ADDR_MATCH_1:
        pmgpmc->pmAddrMatch1 = value;
        break;
      case PM_ADDR_MATCH_2:
        pmgpmc->pmAddrMatch2 = value;
        break;
      case PM_ADDR_MATCH_3:
        pmgpmc->pmAddrMatch3 = value;
        break;
      case PM_ADDR_MATCH_4:
        pmgpmc->pmAddrMatch4 = value;
        break;
      case PM_ADDR_MATCH_5:
        pmgpmc->pmAddrMatch5 = value;
        break;
      case PM_ADDR_MATCH_6:
        pmgpmc->pmAddrMatch6 = value;
        break;
      case PM_ADDR_MATCH_7:
        pmgpmc->pmAddrMatch7 = value;
        break;
      default:
        printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x, value %#.8x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size, value);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_OCM_RAM && phyAddr < PM_OCM_ROM)
  {
    regOffset = phyAddr - PM_OCM_RAM;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        pmocmram->pmErrorLog = value;
        break;
      case PM_CONTROL:
        pmocmram->pmControl = value;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        pmocmram->pmReqInfoPermission0 = value;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        pmocmram->pmReqInfoPermission1 = value;
        break;
      case PM_REQ_INFO_PERMISSION_2:
        pmocmram->pmReqInfoPermission2 = value;
        break;
      case PM_REQ_INFO_PERMISSION_3:
        pmocmram->pmReqInfoPermission3 = value;
        break;
      case PM_REQ_INFO_PERMISSION_4:
        pmocmram->pmReqInfoPermission4 = value;
        break;
      case PM_REQ_INFO_PERMISSION_5:
        pmocmram->pmReqInfoPermission5 = value;
        break;
      case PM_REQ_INFO_PERMISSION_6:
        pmocmram->pmReqInfoPermission6 = value;
        break;
      case PM_REQ_INFO_PERMISSION_7:
        pmocmram->pmReqInfoPermission7 = value;
        break;
      case PM_READ_PERMISSION_0:
        pmocmram->pmReadPermission0 = value;
        break;
      case PM_READ_PERMISSION_1:
        pmocmram->pmReadPermission1 = value;
        break;
      case PM_READ_PERMISSION_2:
        pmocmram->pmReadPermission2 = value;
        break;
      case PM_READ_PERMISSION_3:
        pmocmram->pmReadPermission3 = value;
        break;
      case PM_READ_PERMISSION_4:
        pmocmram->pmReadPermission4 = value;
        break;
      case PM_READ_PERMISSION_5:
        pmocmram->pmReadPermission5 = value;
        break;
      case PM_READ_PERMISSION_6:
        pmocmram->pmReadPermission6 = value;
        break;
      case PM_READ_PERMISSION_7:
        pmocmram->pmReadPermission7 = value;
        break;
      case PM_WRITE_PERMISSION_0:
        pmocmram->pmWritePermission0 = value;
        break;
      case PM_WRITE_PERMISSION_1:
        pmocmram->pmWritePermission1 = value;
        break;
      case PM_WRITE_PERMISSION_2:
        pmocmram->pmWritePermission2 = value;
        break;
      case PM_WRITE_PERMISSION_3:
        pmocmram->pmWritePermission3 = value;
        break;
      case PM_WRITE_PERMISSION_4:
        pmocmram->pmWritePermission4 = value;
        break;
      case PM_WRITE_PERMISSION_5:
        pmocmram->pmWritePermission5 = value;
        break;
      case PM_WRITE_PERMISSION_6:
        pmocmram->pmWritePermission6 = value;
        break;
      case PM_WRITE_PERMISSION_7:
        pmocmram->pmWritePermission7 = value;
        break;
      case PM_ADDR_MATCH_1:
        pmocmram->pmAddrMatch1 = value;
        break;
      case PM_ADDR_MATCH_2:
        pmocmram->pmAddrMatch2 = value;
        break;
      case PM_ADDR_MATCH_3:
        pmocmram->pmAddrMatch3 = value;
        break;
      case PM_ADDR_MATCH_4:
        pmocmram->pmAddrMatch4 = value;
        break;
      case PM_ADDR_MATCH_5:
        pmocmram->pmAddrMatch5 = value;
        break;
      case PM_ADDR_MATCH_6:
        pmocmram->pmAddrMatch6 = value;
        break;
      case PM_ADDR_MATCH_7:
        pmocmram->pmAddrMatch7 = value;
        break;
      default:
        printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x, value %#.8x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size, value);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_OCM_ROM && phyAddr < PM_MAD2D)
  {
    regOffset = phyAddr - PM_OCM_ROM;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        pmocmrom->pmErrorLog = value;
        break;
      case PM_CONTROL:
        pmocmrom->pmControl = value;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        pmocmrom->pmReqInfoPermission0 = value;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        pmocmrom->pmReqInfoPermission1 = value;
        break;
      case PM_READ_PERMISSION_0:
        pmocmrom->pmReadPermission0 = value;
        break;
      case PM_READ_PERMISSION_1:
        pmocmrom->pmReadPermission1 = value;
        break;
      case PM_WRITE_PERMISSION_0:
        pmocmrom->pmWritePermission0 = value;
        break;
      case PM_WRITE_PERMISSION_1:
        pmocmrom->pmWritePermission1 = value;
        break;
      case PM_ADDR_MATCH_1:
        pmocmrom->pmAddrMatch1 = value;
        break;
      default:
        printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x, value %#.8x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size, value);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else if (phyAddr >= PM_IVA2 && phyAddr < (Q1_L3_PM + Q1_L3_PM_SIZE))
  {
    regOffset = phyAddr - PM_IVA2;

    switch (regOffset)
    {
      case PM_ERROR_LOG:
        pmiva->pmErrorLog = value;
        break;
      case PM_CONTROL:
        pmiva->pmControl = value;
        break;
      case PM_REQ_INFO_PERMISSION_0:
        pmiva->pmReqInfoPermission0 = value;
        break;
      case PM_REQ_INFO_PERMISSION_1:
        pmiva->pmReqInfoPermission1 = value;
        break;
      case PM_REQ_INFO_PERMISSION_2:
        pmiva->pmReqInfoPermission2 = value;
        break;
      case PM_REQ_INFO_PERMISSION_3:
        pmiva->pmReqInfoPermission3 = value;
        break;
      case PM_READ_PERMISSION_0:
        pmiva->pmReadPermission0 = value;
        break;
      case PM_READ_PERMISSION_1:
        pmiva->pmReadPermission1 = value;
        break;
      case PM_READ_PERMISSION_2:
        pmiva->pmReadPermission2 = value;
        break;
      case PM_READ_PERMISSION_3:
        pmiva->pmReadPermission3 = value;
        break;
      case PM_WRITE_PERMISSION_0:
        pmiva->pmWritePermission0 = value;
        break;
      case PM_WRITE_PERMISSION_1:
        pmiva->pmWritePermission1 = value;
        break;
      case PM_WRITE_PERMISSION_2:
        pmiva->pmWritePermission2 = value;
        break;
      case PM_WRITE_PERMISSION_3:
        pmiva->pmWritePermission3 = value;
        break;
      case PM_ADDR_MATCH_1:
        pmiva->pmAddrMatch1 = value;
        break;
      case PM_ADDR_MATCH_2:
        pmiva->pmAddrMatch2 = value;
        break;
      case PM_ADDR_MATCH_3:
        pmiva->pmAddrMatch3 = value;
        break;
      default:
        printf("%s store to pAddr: %#.8x, vAddr: %#.8x, accSize %x, value %#.8x" EOL, dev->deviceName, phyAddr,
            virtAddr, (u32int)size, value);
        DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
        break;
    }
  }
  else
  {
    DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }
}

