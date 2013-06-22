#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/i2c.h"
#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/twl4030.h"

#include "memoryManager/pageTable.h"

struct I2c *i2c[3];

u32int getI2cId(u32int address);

u32int getI2cId(u32int address)
{
  switch (address & 0xFFFFFF00)
  {
    case I2C1:
      return 0;
    case I2C2:
      return 1;
    case I2C3:
      return 2;
    default:
      DIE_NOW(NULL, "Invalid i2c device.");
  }
}

void initI2c(virtualMachine *vm, u32int i2cNumber)
{
  // TODO
  i2c[i2cNumber -1] = (struct I2c *)calloc(1,sizeof(struct I2c));
  if (!i2c[i2cNumber -1])
  {
    DIE_NOW(NULL, "Failed to allocate I2c.");
  }

  memset(i2c[i2cNumber -1], 0, sizeof(struct I2c));

  printf ("i2c%d initialised\n", i2cNumber);
  
  if (i2cNumber == 1)
  {
    initTwl4030(0);
  }
}

u32int loadI2c(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val;
  int id = getI2cId(phyAddr);

  switch (phyAddr & 0xFF)
  {
    case I2C_REV:
      val = I2C_REV_BEAGLE;
      break;
    case I2C_SYSS:
      val = i2c[id]->i2cSyss;
      break;
    case I2C_IE:
      val = i2c[id]->i2cIe;
      break;
    case I2C_CON:
      val = i2c[id]->i2cCon;
      break;
    case I2C_STAT:
      val = i2c[id]->i2cStat;
      break;
    case I2C_BUF:
      val = i2c[id]->i2cBuf;
      break;
    case I2C_BUFSTAT:
      val = i2c[id]->i2cBufstat;
      break;
    case I2C_DATA:
      switch (i2c[id]->i2cSa)
      {
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
          val = twl4030_read();
          break;
        default:
          val = 0xff;
          printf("Trying to read from unknown i2c slave: 0x%x", i2c[id]->i2cSa);
          DIE_NOW(NULL, "debug me\n");
      }

      if (--i2c[id]->i2cCnt == 0)
      {
        i2c[id]->i2cStat = I2C_ARDY;
        i2c[id]->i2cCon &= ~I2C_STP;
      }
      break;
    default:
      printf("i2c trying to read from unimplemented register: 0x%x\n", phyAddr);
      DIE_NOW(NULL, "debug me\n");
  }

  DEBUG(VP_OMAP_35XX_I2C, "i2c read 0x%x from 0x%x\n", val, phyAddr);
  return val;
}


void storeI2c(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_I2C, "i2c store 0x%x to 0x%x\n", value, phyAddr);

  int id = getI2cId(phyAddr);

  switch (phyAddr & 0xFF)
  {
    case I2C_SYSC:
      if (value & I2C_SRST)
      {
        i2c[id]->i2cSysc = value;
        i2c[id]->i2cSyss |= I2C_RDONE;
      }
      break;
    case I2C_CON:
      i2c[id]->i2cCon = value;
      i2c[id]->i2cStat &= I2C_INT_EV;
      // if start condition asserted, start
      if (value & I2C_STT)
      {
        // slave must be configured correctly
        switch(i2c[id]->i2cSa)
        {
          case 0x48:
          case 0x49:
          case 0x4a:
          case 0x4b:
            twl4030_start(i2c[id]->i2cSa, !(i2c[id]->i2cCon & I2C_TRX));
            break;
          default:
            printf("Trying to start transfer with unknown i2c slave: 0x%x", i2c[id]->i2cSa);
            DIE_NOW(NULL, "debug me\n");
        }

        i2c[id]->i2cCon &= ~I2C_STT;

        if (i2c[id]->i2cCon & I2C_TRX) 
        {
          i2c[id]->i2cStat = I2C_XDR | I2C_XUDF;
          i2c[id]->i2cBufstat = i2c[id]->i2cCnt;
        }
        else
        {
          i2c[id]->i2cStat = I2C_RDR;
          i2c[id]->i2cBufstat = i2c[id]->i2cCnt << 8;
        }

        DEBUG(VP_OMAP_35XX_I2C, "i2c Throwing interrupt (start)\n");
        switch (id)
        {
          case 0:
            throwInterrupt(context, I2C1_IRQ);
            break;
          case 1:
            throwInterrupt(context, I2C2_IRQ);
            break;
          case 2:
            throwInterrupt(context, I2C3_IRQ);
            break;
        }
      }
      if (value & I2C_STP)
      {
        switch(i2c[id]->i2cSa)
        {
          case 0x48:
          case 0x49:
          case 0x4a:
          case 0x4b:
            twl4030_stop();
            break;
          default:
            printf("Trying to stop transfer with unknown i2c slave: 0x%x", i2c[id]->i2cSa);
            DIE_NOW(NULL, "debug me\n");
        }

        i2c[id]->i2cCon &= ~I2C_STP;
        i2c[id]->i2cStat = I2C_ARDY;

        DEBUG(VP_OMAP_35XX_I2C, "Throwing interrupt (stop)\n");
        switch (id)
        {
          case 0:
            throwInterrupt(context, I2C1_IRQ);
            break;
          case 1:
            throwInterrupt(context, I2C2_IRQ);
            break;
          case 2:
            throwInterrupt(context, I2C3_IRQ);
            break;
        }
      }
      break;
    case I2C_WE:
      i2c[id]->i2cWe = value;
      break;
    case I2C_PSC:
      i2c[id]->i2cPsc = value;
      break;
    case I2C_SCLL:
      i2c[id]->i2cScll = value;
      break;
    case I2C_SCLH:
      i2c[id]->i2cSclh = value;
      break;
    case I2C_BUF:
      i2c[id]->i2cBuf = value & ~(I2C_RDMA_EN | I2C_XDMA_EN);
      break;
    case I2C_IE:
      i2c[id]->i2cIe = value;
      break;
    case I2C_STAT:
      i2c[id]->i2cStat &= ~value;
      break;
    case I2C_SA: // slave address
      i2c[id]->i2cSa = value;
      break;
    case I2C_CNT: // data count
      i2c[id]->i2cCnt = value;
      break;
    case I2C_DATA:
      switch (i2c[id]->i2cSa)
      {
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4b:
          twl4030_write(value);
          break;
        default:
          printf("WARNING: Trying to write 0x%x to unknown i2c slave: 0x%x\n", value, i2c[id]->i2cSa);
          DIE_NOW(NULL, "debug me\n");
      }

      if (--i2c[id]->i2cCnt == 0)
      {      
        i2c[id]->i2cStat = I2C_ARDY;
        i2c[id]->i2cCon &= ~I2C_STP;
      }
      
      break;
    default:
      printf("i2c trying to write 0x%x to unimplemented register: 0x%x\n", value, phyAddr);
      DIE_NOW(NULL, "debug me\n");
  }
}
