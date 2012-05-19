#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/gpmc.h"



void initGpmc(virtualMachine *vm)
{
  struct Gpmc *gpmc = (struct Gpmc *)calloc(1, sizeof(struct Gpmc));
  if (gpmc == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate GPMC.");
  }

  DEBUG(VP_OMAP_35XX_GPMC, "initGpmc: @ %p" EOL, gpmc);

  // register default values
  gpmc->gpmcSysConfig = 0x00000010; // OMAP reference manual: 0x00000000
  gpmc->gpmcSysStatus = 0x00000001;
  gpmc->gpmcIrqStatus = 0x00000100; // OMAP reference manual: 0x00000000
  gpmc->gpmcIrqEnable = 0x00000000;
  gpmc->gpmcTimeoutControl = 0x00000000; // OMAP reference manual: 0x00001ff0
  gpmc->gpmcErrAddress = 0x00000000;
  gpmc->gpmcErrType = 0x00000000;
  gpmc->gpmcConfig = 0x00000000; // OMAP reference manual: 0x00000a00
  gpmc->gpmcStatus = 0x00000001;

  gpmc->gpmcConfig1_0 = 0x00001800; // dev size 16bits, nand flash like device
  gpmc->gpmcConfig7_0 = 0x00000f00; // CS0 off; OMAP reference manual: 0x00000f40 (turns CS0 on)
  gpmc->gpmcConfig1_1 = 0x00001000; // dev size 16bits, nor flash like device
  gpmc->gpmcConfig7_1 = 0x00000f00;
  gpmc->gpmcConfig1_2 = 0x00001000; // dev size 16bits, nor flash like device
  gpmc->gpmcConfig7_2 = 0x00000f00;
  gpmc->gpmcConfig1_3 = 0x00001000; // dev size 16bits, nor flash like device
  gpmc->gpmcConfig7_3 = 0x00000f00;
  gpmc->gpmcConfig1_4 = 0x00001000; // dev size 16bits, nor flash like device
  gpmc->gpmcConfig7_4 = 0x00000f00;
  gpmc->gpmcConfig1_5 = 0x00001000; // dev size 16bits, nor flash like device
  gpmc->gpmcConfig7_5 = 0x00000f00;
  gpmc->gpmcConfig1_6 = 0x00001000; // dev size 16bits, nor flash like device
  gpmc->gpmcConfig7_6 = 0x00000f00;
  gpmc->gpmcConfig1_7 = 0x00001000; // dev size 16bits, nor flash like device
  gpmc->gpmcConfig7_7 = 0x00000f00;

  gpmc->gpmcPrefetchConfig1 = 0x00000000; // OMAP reference manual: 0x00004000
  gpmc->gpmcPrefetchConfig2 = 0x00000000;
  gpmc->gpmcPrefetchControl = 0x00000000;
  gpmc->gpmcPrefetchStatus = 0x00000000;
  gpmc->gpmcEccConfig = 0x00000000; // OMAP reference manual: 0x00001030
  gpmc->gpmcEccControl = 0x00000000;
  gpmc->gpmcEccSizeConfig = 0x00000000; // OMAP reference manual: 0x3fcff000
  // TODO: add rest

  vm->gpmc = gpmc;
}

/* top load function */
u32int loadGpmc(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "Gpmc: invalid access size.");
  }

  GCONTXT* context = getGuestContext();
  struct Gpmc* gpmc = context->vm.gpmc;

  u32int regOffset = phyAddr - Q1_L3_GPMC;
  u32int val = 0;
  switch (regOffset)
  {
    case GPMC_REVISION:
      val = GPMC_REVISION_VALUE;
      break;
    case GPMC_SYSCONFIG:
      // TODO
      printf("WARN reading GPMC_SYSCONFIG %#.8x" EOL, gpmc->gpmcSysConfig);
      val = gpmc->gpmcSysConfig;
      break;
    case GPMC_NAND_COMMAND_0:
    case GPMC_NAND_ADDRESS_0:
    case GPMC_NAND_COMMAND_1:
    case GPMC_NAND_ADDRESS_1:
    case GPMC_NAND_COMMAND_2:
    case GPMC_NAND_ADDRESS_2:
    case GPMC_NAND_COMMAND_3:
    case GPMC_NAND_ADDRESS_3:
    case GPMC_NAND_COMMAND_4:
    case GPMC_NAND_ADDRESS_4:
    case GPMC_NAND_COMMAND_5:
    case GPMC_NAND_ADDRESS_5:
    case GPMC_NAND_COMMAND_6:
    case GPMC_NAND_ADDRESS_6:
    case GPMC_NAND_COMMAND_7:
    case GPMC_NAND_ADDRESS_7:
      DIE_NOW(NULL, "Gpmc: load on write-only register.");
      break;
    case GPMC_CONFIG1_0:
      val = gpmc->gpmcConfig1_0;
      break;
    case GPMC_CONFIG7_0:
      val = gpmc->gpmcConfig7_0;
      break;
    case GPMC_CONFIG1_1:
      val = gpmc->gpmcConfig1_1;
      break;
    case GPMC_CONFIG7_1:
      val = gpmc->gpmcConfig7_1;
      break;
    case GPMC_CONFIG1_2:
      val = gpmc->gpmcConfig1_2;
      break;
    case GPMC_CONFIG7_2:
      val = gpmc->gpmcConfig7_2;
      break;
    case GPMC_CONFIG1_3:
      val = gpmc->gpmcConfig1_3;
      break;
    case GPMC_CONFIG7_3:
      val = gpmc->gpmcConfig7_3;
      break;
    case GPMC_CONFIG1_4:
      val = gpmc->gpmcConfig1_4;
      break;
    case GPMC_CONFIG7_4:
      val = gpmc->gpmcConfig7_4;
      break;
    case GPMC_CONFIG1_5:
      val = gpmc->gpmcConfig1_5;
      break;
    case GPMC_CONFIG7_5:
      val = gpmc->gpmcConfig7_5;
      break;
    case GPMC_CONFIG1_6:
      val = gpmc->gpmcConfig1_6;
      break;
    case GPMC_CONFIG7_6:
      val = gpmc->gpmcConfig7_6;
      break;
    case GPMC_CONFIG1_7:
      val = gpmc->gpmcConfig1_7;
      break;
    case GPMC_CONFIG7_7:
      val = gpmc->gpmcConfig7_7;
      break;
    default:
      printf("%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %x" EOL, dev->deviceName, phyAddr,
          virtAddr, (u32int)size);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }

  DEBUG(VP_OMAP_35XX_GPMC, "%s load from pAddr: %#.8x, vAddr: %#.8x, accSize %#x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  return val;
}


/* top store function */
void storeGpmc(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_GPMC, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  GCONTXT* context = getGuestContext();
  struct Gpmc* gpmc = context->vm.gpmc;

  switch (phyAddr - Q1_L3_GPMC)
  {
    case GPMC_SYSCONFIG:
    {
      // TODO
      printf("WARN writing to GPMC_SYSCONFIG %#.8x" EOL, value);
      if (value & GPMC_SYSCONFIG_SOFTRESET)
      {
        printf("WARN should do soft reset of GPMC" EOL);
      }
      gpmc->gpmcSysConfig = value & GPMC_SYSCONFIG_MASK;
      break;
    }
    case GPMC_SYSSTATUS:
      DIE_NOW(NULL, "store to read-only register");
      break;
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}
