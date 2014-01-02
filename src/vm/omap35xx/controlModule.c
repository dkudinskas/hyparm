#include "common/debug.h"
#include "common/stddef.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/controlModule.h"
#include "vm/omap35xx/controlModuleInternals.h"


u32int loadControlModule(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  const u32int registerOffset = phyAddr - CONTROL_MODULE_ID;
  u32int value = 0;
  switch (registerOffset)
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
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
  printf("loadControlModule: regOffs %x value %08x\n", registerOffset, value);
  return value;
}

void storeControlModule(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DIE_NOW(NULL, "read only device");
}
