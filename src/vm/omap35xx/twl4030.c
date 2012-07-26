#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/i2c.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/twl4030.h"
#include "vm/omap35xx/mmc.h"

#include "memoryManager/pageTable.h"

struct Twl4030 * twl4030;

void initTwl4030(int no)
{
  twl4030 = (struct Twl4030 *)calloc(1, sizeof(struct Twl4030));

  if (!twl4030)
  {
    DIE_NOW(NULL, "Failed to allocate Twl4030.");
  }

  memset(twl4030, 0, sizeof(struct Twl4030));
}

void twl4030_start(u8int address, int read)
{
  switch (twl4030->state)
  {
    case WRITE1:
      if (read)
        twl4030->state = READ1;
      else 
        twl4030->state = WRITE2;
      break;
    case IDLE:
      if (read) 
      {
        DIE_NOW(NULL, "twl4030: can't go from idle to read state");
      }
      else
      {
        twl4030->selected_group = address;
        twl4030->state = SELECT;
      }
      break;
    default:
      DIE_NOW(NULL, "twl4030 broken i2c transaction");
  }
}

void twl4030_stop()
{
  twl4030->state = IDLE;
}

u8int twl4030_read()
{
  u8int val;

  if (twl4030->state != READ1)
  {
    DIE_NOW(NULL, "twl4030 read request while in different state");
  }

  val = twl4030->registers[twl4030->selected_group - TWL_GRP_OFFSET][twl4030->selected_reg];

  switch(twl4030->selected_group)
  {
    case 0x49:
      switch (twl4030->selected_reg)
      {
        case TWL_GPIODATAIN1:
          val = mmc[0]->cardPresent ? (val & ~TWL_GPIO1_HIGH) : (val | TWL_GPIO1_HIGH);
          break;
      }
      break;
  }

  DEBUG(VP_OMAP_35XX_TWL4030, "twl4030 read 0x%x, 0x%x: 0x%x\n", twl4030->selected_group,
        twl4030->selected_reg, val);

  twl4030->selected_reg++;

  return val;
}

void twl4030_write(u8int value)
{
  if (twl4030->state != SELECT && twl4030->state != WRITE1 && twl4030->state != WRITE2)
  {
    DIE_NOW(NULL, "twl4030 write request while in different state");
  }

  if (twl4030->state == SELECT)
  { 
    DEBUG(VP_OMAP_35XX_TWL4030, "twl4030 select register 0x%x from group 0x%x\n", value, twl4030->selected_group);
  
    twl4030->selected_reg = value;
    twl4030->state = WRITE1;
  }
  else
  {
    if (twl4030->state == WRITE1)
    {
      twl4030->state = WRITE2;
    }
    DEBUG(VP_OMAP_35XX_TWL4030, "twl4030 write 0x%x to 0x%x, 0x%x\n", value, twl4030->selected_group,
          twl4030->selected_reg);

    // Implement any logic here

    twl4030->registers[twl4030->selected_group - TWL_GRP_OFFSET][twl4030->selected_reg++] = value;
  }
}
