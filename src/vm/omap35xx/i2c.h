#ifndef __VM__OMAP_35XX__I2C_H__
#define __VM__OMAP_35XX__I2C_H__

void initI2c(virtualMachine *vm, u32int i2cNumber);
u32int loadI2c(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeI2c(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#define I2C_REV     0x00
#define I2C_IE      0x04
#define I2C_STAT    0x08
#define I2C_WE      0x0C
#define I2C_SYSS    0x10
#define I2C_BUF     0x14
#define I2C_CNT     0x18
#define I2C_DATA    0x1C
#define I2C_SYSC    0x20
#define I2C_CON     0x24
#define I2C_OA0     0x28
#define I2C_SA      0x2C
#define I2C_PSC     0x30
#define I2C_SCLL    0x34
#define I2C_SCLH    0x38
#define I2C_SYSTEST 0x3C
#define I2C_BUFSTAT 0x40
#define I2C_OA1     0x44
#define I2C_OA2     0x48
#define I2C_OA3     0x4C
#define I2C_ACTOA   0x50
#define I2C_SBLOCK  0x54

#define I2C_SRST    0x2
#define I2C_RDONE   0x1
#define I2C_TRX     (1 << 9)
#define I2C_XRDY    (1 << 4)
#define I2C_RRDY    (1 << 3)
#define I2C_XDR     (1 << 14)
#define I2C_RDR     (1 << 13)
#define I2C_ARDY    (1 << 2)
#define I2C_XUDF    (1 << 10)

#define I2C_INT_EV  (I2C_XRDY | I2C_RRDY | I2C_ARDY | I2C_XUDF)

#define I2C_RDMA_EN (1 << 15)
#define I2C_XDMA_EN (1 << 7)

#define I2C_STT     (1 << 0)
#define I2C_STP     (1 << 1)

#define I2C_REV_BEAGLE 0x003c003c

struct I2c
{
  u32int i2cRev;
  u32int i2cIe;
  u32int i2cStat;
  u32int i2cWe;
  u32int i2cSyss;
  u32int i2cBuf;
  u32int i2cCnt;
  u32int i2cData;
  u32int i2cSysc;
  u32int i2cCon;
  u32int i2cOa0;
  u32int i2cSa;
  u32int i2cPsc;
  u32int i2cScll;
  u32int i2cSclh;
  u32int i2cSystest;
  u32int i2cBufstat;
  u32int i2cOa1;
  u32int i2cOa2;
  u32int i2cOa3;
  u32int i2cActoa;
  u32int i2cSblock;
};

#endif /* __VM__OMAP_35XX__I2C_H__ */
