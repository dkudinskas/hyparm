#include "common/debug.h"
#include "common/stddef.h"

#include "guestManager/guestContext.h"

#include "controlModule.h"

void initControlModule()
{
  // nothing to initialise.
  return;
}

u32int loadControlModule(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int value = 0;
  u32int regOffs = phyAddr - CONTROL_MODULE_ID;
  switch (regOffs)
  {
    case CONTROL_MOD_IDCODE:
    {
      value = CONTROL_MOD_IDCODE_VALUE;
      break;
    }
    case CONTROL_MOD_PROD_ID:
    {
      value = CONTROL_MOD_PROD_ID_720_520;
      break;
    }
    case CONTROL_MOD_RESERVED:
    {
      value = CONTROL_MOD_RESERVED_VALUE;
      break;
    }
    default:
    {
      printf("loadControlModule: reg offs %x\n", regOffs);
      DIE_NOW(0, "loadControlModule: unimplemented.");
    }
  }
  printf("loadControlModule: regOffs %x value %08x\n", regOffs, value);
  return value;
}

void storeControlModule(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DIE_NOW(0, "storeControlModule: control module is a read only device!");
}
