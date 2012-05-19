#include "common/debug.h"
#include "common/stddef.h"

#include "guestManager/guestContext.h"

#include "controlModule.h"


/**
 * IMPORTANT:
 * Linux kernel arch/arm/mach-omap2/id.c
 * really misidentifies more current OMAP revisions, thus linux register definitions
 * are completely wrong.
 **/

#define CONTROL_MOD_IDCODE            0x204
#define CONTROL_MOD_IDCODE_VALUE      0x4B7AE02F
#define CONTROL_MOD_RESERVED          0x208
#define CONTROL_MOD_RESERVED_VALUE    0x00000000
#define CONTROL_MOD_PROD_ID           0x20c
#define CONTROL_MOD_PROD_ID_600_430   0x00000000
#define CONTROL_MOD_PROD_ID_720_520   0x00000008


u32int loadControlModule(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
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

void storeControlModule(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DIE_NOW(NULL, "read only device");
}
