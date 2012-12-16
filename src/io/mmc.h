#ifndef __MMC_H__
#define __MMC_H__

#include "common/types.h"

#include "io/block.h"


#define SD_VERSION_SD 0x20000
#define SD_VERSION_2  (SD_VERSION_SD | 0x20)
#define SD_VERSION_1_0  (SD_VERSION_SD | 0x10)
#define SD_VERSION_1_10 (SD_VERSION_SD | 0x1a)
#define MMC_VERSION_MMC   0x10000
#define MMC_VERSION_UNKNOWN (MMC_VERSION_MMC)
#define MMC_VERSION_1_2   (MMC_VERSION_MMC | 0x12)
#define MMC_VERSION_1_4   (MMC_VERSION_MMC | 0x14)
#define MMC_VERSION_2_2   (MMC_VERSION_MMC | 0x22)
#define MMC_VERSION_3   (MMC_VERSION_MMC | 0x30)
#define MMC_VERSION_4   (MMC_VERSION_MMC | 0x40)

#define MMC_MODE_HS   0x001
#define MMC_MODE_HS_52MHz 0x010
#define MMC_MODE_4BIT   0x100
#define MMC_MODE_8BIT   0x200

#define SD_DATA_4BIT  0x00040000

#define IS_SD(x) (x->version & SD_VERSION_SD)

#define MMC_DATA_READ     1
#define MMC_DATA_WRITE    2

#define NO_CARD_ERR   -16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR    -17 /* Unusable Card */
#define COMM_ERR    -18 /* Communications Error */
#define TIMEOUT     -19

#define MMC_CMD_GO_IDLE_STATE   0
#define MMC_CMD_SEND_OP_COND    1
#define MMC_CMD_ALL_SEND_CID    2
#define MMC_CMD_SET_RELATIVE_ADDR 3
#define MMC_CMD_SET_DSR     4
#define MMC_CMD_SWITCH      6
#define MMC_CMD_SELECT_CARD   7
#define MMC_CMD_SEND_EXT_CSD    8
#define MMC_CMD_SEND_CSD    9
#define MMC_CMD_SEND_CID    10
#define MMC_CMD_STOP_TRANSMISSION 12
#define MMC_CMD_SEND_STATUS   13
#define MMC_CMD_SET_BLOCKLEN    16
#define MMC_CMD_READ_SINGLE_BLOCK 17
#define MMC_CMD_READ_MULTIPLE_BLOCK 18
#define MMC_CMD_WRITE_SINGLE_BLOCK  24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK  25
#define MMC_CMD_APP_CMD     55

#define SD_CMD_SEND_RELATIVE_ADDR 3
#define SD_CMD_SWITCH_FUNC    6
#define SD_CMD_SEND_IF_COND   8

#define SD_CMD_APP_SET_BUS_WIDTH  6
#define SD_CMD_APP_SEND_OP_COND   41
#define SD_CMD_APP_SEND_SCR   51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY 0x00020000
#define SD_HIGHSPEED_SUPPORTED  0x00020000

#define MMC_HS_TIMING   0x00000100
#define MMC_HS_52MHZ    0x2

#define OCR_BUSY  0x80000000
#define OCR_HCS   0x40000000

#define MMC_VDD_165_195   0x00000080  /* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21   0x00000100  /* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22   0x00000200  /* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23   0x00000400  /* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24   0x00000800  /* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25   0x00001000  /* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26   0x00002000  /* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27   0x00004000  /* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28   0x00008000  /* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29   0x00010000  /* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30   0x00020000  /* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31   0x00040000  /* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32   0x00080000  /* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33   0x00100000  /* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34   0x00200000  /* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35   0x00400000  /* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36   0x00800000  /* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET   0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS  0x01 /* Set bits in EXT_CSD byte addressed
                                          by index which are
                                          1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS  0x02 /* Clear bits in EXT_CSD byte
                                            addressed by index, which are
                                            1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE  0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK   0
#define SD_SWITCH_SWITCH  1

/*
 * EXT_CSD fields
 */
#define EXT_CSD_BUS_WIDTH 183 /* R/W */
#define EXT_CSD_HS_TIMING 185 /* R/W */
#define EXT_CSD_CARD_TYPE 196 /* RO */
#define EXT_CSD_REV   192 /* RO */
#define EXT_CSD_SEC_CNT   212 /* RO, 4 bytes */

/*
 * EXT_CSD field definitions
 */
#define EXT_CSD_CMD_SET_NORMAL    (1<<0)
#define EXT_CSD_CMD_SET_SECURE    (1<<1)
#define EXT_CSD_CMD_SET_CPSECURE  (1<<2)

#define EXT_CSD_CARD_TYPE_26  (1<<0)  /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52  (1<<1)  /* Card can run at 52MHz */

#define EXT_CSD_BUS_WIDTH_1 0 /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4 1 /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8 2 /* Card is in 8 bit mode */

#define R1_ILLEGAL_COMMAND    (1 << 22)
#define R1_APP_CMD      (1 << 5)

#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136     (1 << 1)                /* 136 bit response */
#define MMC_RSP_CRC     (1 << 2)                /* expect valid crc */
#define MMC_RSP_BUSY    (1 << 3)                /* card may send busy */
#define MMC_RSP_OPCODE  (1 << 4)                /* response contains opcode */

#define MMC_RSP_NONE    (0)
#define MMC_RSP_R1      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
      MMC_RSP_BUSY)
#define MMC_RSP_R2      (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3      (MMC_RSP_PRESENT)
#define MMC_RSP_R4      (MMC_RSP_PRESENT)
#define MMC_RSP_R5      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)



/* This code is based on the MMC protocol code from the u-boot project */
struct mmcCommand
{
  u16int idx;
  u32int responseType;
  u32int arg;
  u32int resp[4];
  u32int flags;
};

struct mmcData
{
  union
  {
    char *dest;
    const char *src;
  };
  u32int flags;
  u32int blocks;
  u32int blocksize;
};

struct mmc
{
  u32int base;
  u32int frequencyMin;
  u32int frequencyMax;
  u32int busWidth;
  u32int clock;
  u32int capacity;
  u32int voltages;
  u32int hostCaps;
  u32int cardCaps;
  u32int version;
  u32int ocr;
  u32int scr[2];
  u32int csd[4];
  u32int cid[4];
  u16int rca;
  u32int transmitSpeed;
  u32int readBlockLength;
  u32int writeBlockLength;
  int highCapacity;
  int (*init)(struct mmc *mmc);
  int (*sendCommand)(struct mmc *mmc, struct mmcCommand *cmd, struct mmcData *data);
  void (*setIOS)(struct mmc *mmc);
  blockDevice blockDev;
};


int mmcMainInit(void);

int mmcRegister(struct mmc *mmc);

u32int mmcBlockRead(int devid, u32int start, u64int blockCount, void *dst);

u32int mmcBlockWrite(int devid, u32int start, u64int blockCount, const void *src);

int mmcSetBlocklen(struct mmc *mmc, int len);

int mmcGoIdle(struct mmc *mmc);

void mmcSetClock(struct mmc *mmc, u32int clock);

void mmcSetBusWidth(struct mmc *mmc, u32int width);

struct mmc *getMMCDevice(int devid);

int sdSwitch(struct mmc *mmc, int mode, int group, u8int val, u8int *resp);

int sdChangeFreq(struct mmc *mmc);

#endif

