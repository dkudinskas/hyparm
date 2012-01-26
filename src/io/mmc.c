#include "common/debug.h"
#include "common/types.h"
#include "common/memFunctions.h"
#include "common/byteOrder.h"

#include "io/mmc.h"

#include "drivers/beagle/be32kTimer.h"

#include "cpuArch/cpu.h"

extern struct mmc *mmcDevice;

u32int mmcRegisteredNumber = 0;

/* MMC block device code derived from the u-boot project */

int mmcRegister(struct mmc *mmc)
{
#ifdef MMC_DEBUG
  printf("Registering mmc device\n");
#endif
  // HACK here, need to redesign this so that getMMCDevice isn't useless
  mmcDevice = mmc; 
  mmc->blockDev.devID = mmcRegisteredNumber;
  mmc->blockDev.blockRead = mmcBlockRead;
  mmc->blockDev.blockWrite = mmcBlockWrite;

  mmcRegisteredNumber++;
  return 0;
}
/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
int fbase[] = {
  10000,
  100000,
  1000000,
  10000000,
};

/* Multiplier values for transmitSpeed.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
int multipliers[] = {
  0,  /* reserved */
  10,
  12,
  13,
  15,
  20,
  25,
  30,
  35,
  40,
  45,
  50,
  55,
  60,
  70,
  80,
};

struct mmc *getMMCDevice(int devid)
{
  //TODO at the moment only support 1 mmc device, so just return that
  return mmcDevice + devid;
}

int mmcSetBlocklen(struct mmc *mmc, int len)
{
  struct mmcCommand cmd;

  cmd.idx = MMC_CMD_SET_BLOCKLEN;
  cmd.responseType = MMC_RSP_R1;
  cmd.arg = len;
  cmd.flags = 0;

  return mmc->sendCommand(mmc, &cmd, 0);
}

/* Main block read function, not called directly but instead 
   is registered with a block device via a function ptr. */
u32int mmcBlockRead(int devid, u32int start, u64int blockCount, void *dst)
{
  // we use devid because the block layer doesn't know about the mmc device,
  // only has an index
  struct mmc *mmc = getMMCDevice(devid);

  if ((start + blockCount) > mmc->blockDev.lba)
  {
    printf("mmcBlockRead: exceeded max block address\n");
    printf("start = %x\n", start);
    printf("blockCount = %x\n", blockCount);
    printf("lba = %x\n", mmc->blockDev.lba);
    return 0;
  }

  //ignore the 16 bit counter case unless we have to deal with it in practice
  struct mmcCommand cmd;
  struct mmcData data;

  if (blockCount > 1)
  {
    cmd.idx = MMC_CMD_READ_MULTIPLE_BLOCK;
  }
  else
  {
    cmd.idx = MMC_CMD_READ_SINGLE_BLOCK;
  }

  if (mmc->highCapacity)
  {
    cmd.arg = start;
  }
  else
  {
    cmd.arg = start * mmc->readBlockLength;
  }

  cmd.responseType = MMC_RSP_R1;
  cmd.flags = 0;

  data.dest = dst;
  data.blocks = blockCount;
  data.blocksize = mmc->readBlockLength;
  data.flags = MMC_DATA_READ;

  if (mmc->sendCommand(mmc, &cmd, &data))
  {
    return 0;
  }

  if (blockCount > 1)
  {
    cmd.idx = MMC_CMD_STOP_TRANSMISSION;
    cmd.arg = 0;
    cmd.responseType = MMC_RSP_R1b;
    cmd.flags = 0;
    
    if (mmc->sendCommand(mmc, &cmd, 0))
    {
      printf("mmcBlockRead: stop cmd failed\n");
      return 0;
    }
  }

  return blockCount;
}


/* Main block write function, not called directly but instead
   is registered with a block device via a function ptr. */
u32int mmcBlockWrite(int devid, u32int start, u64int blockCount, const void *src)
{
  struct mmc *mmc = getMMCDevice(devid);

  //ignore the 16 bit block counter constraint for now

  struct mmcCommand cmd;
  struct mmcData data;

  if ((start + blockCount) > mmc->blockDev.lba)
  {
    printf("mmcBlockWrite: exceeded max block address\n");
    return 0;
  }

  if (blockCount > 1)
  {
    cmd.idx = MMC_CMD_WRITE_MULTIPLE_BLOCK;
  }
  else
  {
    cmd.idx = MMC_CMD_WRITE_SINGLE_BLOCK;
  }

  if (mmc->highCapacity)
  {
    cmd.arg = start;
  }
  else
  {
    cmd.arg = start * mmc->writeBlockLength;
  }
  
  cmd.responseType = MMC_RSP_R1;
  cmd.flags = 0;

  data.src = src;
  data.blocks = blockCount;
  data.blocksize = mmc->writeBlockLength;
  data.flags = MMC_DATA_WRITE;

  if (mmc->sendCommand(mmc, &cmd, &data))
  {
    printf("mmcBlockWrite: sendCommand failed\n");
    return 0;
  }

  if (blockCount > 1)
  {
    cmd.idx = MMC_CMD_STOP_TRANSMISSION;
    cmd.arg = 0;
    cmd.responseType = MMC_RSP_R1b;
    cmd.flags = 0;

    if (mmc->sendCommand(mmc, &cmd, 0))
    {
      printf("mmcBlockWrite: stop cmd failed\n");
      return 0;
    }
  }

  return blockCount;
}


int mmcGoIdle(struct mmc *mmc)
{
  struct mmcCommand cmd;
  int error;

  mdelay32k(1);

#ifdef MMC_DEBUG
  printf("mmcGoIdle(): cmd.idx = MMC_CMD_GO_IDLE_STATE\n");
#endif

  cmd.idx = MMC_CMD_GO_IDLE_STATE;
  cmd.arg = 0;
  cmd.responseType = MMC_RSP_NONE;
  cmd.flags = 0;

#ifdef MMC_DEBUG
  printf("mmcGoIdle(): sendCommand\n");
#endif
  if ((error = mmc->sendCommand(mmc, &cmd, 0)))
  {
#ifdef MMC_DEBUG
    printf("mmcGoIdle(): ... failed (errno %x).\n", error);
#endif
    return error;
  }
#ifdef MMC_DEBUG
  printf("mmcGoIdle(): ...done\n");
#endif

  mdelay32k(2);

  return 0;
}

void mmcSetClock(struct mmc *mmc, u32int clock)
{
  if (clock > mmc->frequencyMax)
  {
    clock = mmc->frequencyMax;
  }

  if (clock < mmc->frequencyMin)
  {
    clock = mmc->frequencyMin;
  }

  mmc->clock = clock;
  mmc->setIOS(mmc);
}

void mmcSetBusWidth(struct mmc *mmc, u32int width)
{
  mmc->busWidth = width;
}

int sdSwitch(struct mmc *mmc, int mode, int group, u8int val, u8int *resp)
{
  struct mmcCommand cmd;
  struct mmcData data;

  /* switch freq */
  cmd.idx = SD_CMD_SWITCH_FUNC;
  cmd.responseType = MMC_RSP_R1;
  cmd.arg = (mode << 31) | 0xffffff;
  cmd.arg &= ~(0xf << (group * 4));
  cmd.arg |= val << (group * 4);
  cmd.flags = 0;

  data.dest = (char *)resp;
  data.blocksize = 64;
  data.blocks = 1;
  data.flags = MMC_DATA_READ;

  return mmc->sendCommand(mmc, &cmd, &data);
}


int sdChangeFreq(struct mmc *mmc)
{
  int err;
  struct mmcCommand cmd;
  u32int scr[2];
  u32int switchStatus[16];
  struct mmcData data;
  int timeout;

  mmc->cardCaps = 0;

  /* Read the SCR to find out if this card supports higher speeds */
  cmd.idx = MMC_CMD_APP_CMD;
  cmd.responseType = MMC_RSP_R1;
  cmd.arg = mmc->rca << 16;
  cmd.flags = 0;

  err = mmc->sendCommand(mmc, &cmd, 0);

  if (err)
  {
    return err;
  }

  cmd.idx = SD_CMD_APP_SEND_SCR;
  cmd.responseType = MMC_RSP_R1;
  cmd.arg = 0;
  cmd.flags = 0;

  timeout = 3;

  do
  {
    data.dest = (char *)&scr;
    data.blocksize = 8;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;
  
    err = mmc->sendCommand(mmc, &cmd, &data);
    if (err)
    {
      if (timeout == 0)
      {
        return err;
      }
    }
    else
    {
      break;
    }
  }
  while (timeout--);

  mmc->scr[0] = __be32_to_cpu(scr[0]);
  mmc->scr[1] = __be32_to_cpu(scr[1]);

  switch ((mmc->scr[0] >> 24) & 0xf)
  {
    case 0:
      mmc->version = SD_VERSION_1_0;
      break;
    case 1:
      mmc->version = SD_VERSION_1_10;
      break;
    case 2:
      mmc->version = SD_VERSION_2;
      break;
    default:
      mmc->version = SD_VERSION_1_0;
      break;
  }

  /* Version 1.0 doesn't support switching */
  if (mmc->version == SD_VERSION_1_0)
  {
    return 0;
  }

  timeout = 4;
  while (timeout--)
  {
    if ((err = sdSwitch(mmc, SD_SWITCH_CHECK, 0, 1, (u8int *)&switchStatus)))
    {
      return err;
    }

    /* The high-speed function is busy. Try again */
    if (!(__be32_to_cpu(switchStatus[7]) & SD_HIGHSPEED_BUSY))
    {
      break;
    }
  }

  if (mmc->scr[0] & SD_DATA_4BIT)
  {
    mmc->cardCaps |= MMC_MODE_4BIT;
  }

  /* If high-speed isn't supported, we return */
  if (!(__be32_to_cpu(switchStatus[3]) & SD_HIGHSPEED_SUPPORTED))
  {
    return 0;
  }

  err = sdSwitch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8int *)&switchStatus);

  if (err)
    return err;

  if ((__be32_to_cpu(switchStatus[4]) & 0x0f000000) == 0x01000000)
  {
    mmc->cardCaps |= MMC_MODE_HS;
  }

  return 0;
}

int mmcStartup(struct mmc *mmc)
{
  u32int mult, freq;
  int err = 0;

  struct mmcCommand cmd;

  // put the card in identify mode
  cmd.idx = MMC_CMD_ALL_SEND_CID;
  cmd.responseType = MMC_RSP_R2;
  cmd.arg = 0;
  cmd.flags = 0;

  if ((err = mmc->sendCommand(mmc, &cmd, 0)))
  {
#ifdef MMC_DEBUG
    printf("mmcStartup(): card put in identify mode failed (errno %x)\n", err);
#endif
    return err;
  }
  
  memcpy(mmc->cid, cmd.resp, 16);

  cmd.idx = SD_CMD_SEND_RELATIVE_ADDR;
  cmd.arg = mmc->rca << 16;
  cmd.responseType = MMC_RSP_R6;
  cmd.flags = 0;
  if ((err = mmc->sendCommand(mmc, &cmd, 0)))
  {
#ifdef MMC_DEBUG
    printf("mmcStartup(): send relative address failed (errno %x)\n", err);
#endif
    return err;
  }

  if (IS_SD(mmc))
  {
    mmc->rca = (cmd.resp[0] >> 16) & 0xffff;
  }

  //get card data
  cmd.idx = MMC_CMD_SEND_CSD;
  cmd.responseType = MMC_RSP_R2;
  cmd.arg = mmc->rca << 16;
  cmd.flags = 0;
  if ((err = mmc->sendCommand(mmc, &cmd, 0)))
  {
#ifdef MMC_DEBUG
    printf("mmcStartup(): failed to get card data (errno %x)\n", err);
#endif
    return err;
  }

  mmc->csd[0] = cmd.resp[0];
  mmc->csd[1] = cmd.resp[1];
  mmc->csd[2] = cmd.resp[2];
  mmc->csd[3] = cmd.resp[3];

  if (mmc->version == MMC_VERSION_UNKNOWN)
  {
    //keep it simple, ignore mmc stuff and focus on SD
    printf("MMC card detected, not supported\n");
    return -1;
  }

  /* Not entirely sure what's happening here but no time */
  /* divide frequency by 10, since the mults are 10x bigger */
  freq = fbase[(cmd.resp[0] & 0x7)];
  mult = multipliers[((cmd.resp[0] >> 3) & 0xf)];

  mmc->transmitSpeed = freq * mult;
  mmc->readBlockLength = 1 << ((cmd.resp[1] >> 16) & 0xf);

  if (IS_SD(mmc))
  {
    mmc->writeBlockLength = mmc->readBlockLength;
  }
  else
  {
    mmc->writeBlockLength = 1 << ((cmd.resp[3] >> 22) & 0xf);
  }

  if (mmc->highCapacity)
  {
    u32int csize = ((mmc->csd[1] & 0x3f) << 16) | ((mmc->csd[2] & 0xffff0000) >> 16);
    // memory capacity = (C_SIZE+1) * 512K byte
    mmc->capacity = (csize+1) * 512 * 1024;  
#ifdef MMC_DEBUG
    printf("mmcStartup(): high capacity, csize = %x\n", csize);
#endif
  }
  else
  {
    u32int csize = ((mmc->csd[1] & 0x3ff) << 2) | ((mmc->csd[2] & 0xc0000000) >> 30);
    u32int cmult = (mmc->csd[2] & 0x00038000) >> 15;
    
    u32int mult = 1 << (cmult + 2);
    u32int blockNumber = (csize + 1) * mult;
   
    u32int blockLength = 1 << ((mmc->csd[1] & 0x000f0000) >> 16);
    mmc->capacity = blockNumber * blockLength;

#ifdef MMC_DEBUG
    printf("mmcStartup(): low capacity, csize = %x, cmult = %x\n", csize, cmult);
#endif
  }

#ifdef MMC_DEBUG
  printf("mmcStartup(): mmc capacity = %x\n", mmc->capacity);
#endif

  if (mmc->readBlockLength > 512)
  {
    mmc->readBlockLength = 512;
  }
  if (mmc->writeBlockLength > 512)
  {
    mmc->writeBlockLength = 512;
  }

  //select card & put it in transfer mode
  cmd.idx = MMC_CMD_SELECT_CARD;
  cmd.responseType = MMC_RSP_R1b;
  cmd.arg = mmc->rca << 16;
  cmd.flags = 0;
  if ((err = mmc->sendCommand(mmc, &cmd, 0)))
  {
#ifdef MMC_DEBUG
    printf("mmcStartup(): card put in xfer mode failed (errno %x)\n", err);
#endif
    return err;
  }

  //ignore mmc cards
  if ((err = sdChangeFreq(mmc)))
  {
#ifdef MMC_DEBUG
    printf("mmcStartup(): sdChangeFreg failed (errno %x)\n", err);
#endif
    return err;
  }

  /* restrict card to host capabilities*/
  mmc->cardCaps &= mmc->hostCaps;

  if (IS_SD(mmc))
  {
    if (mmc->cardCaps & MMC_MODE_4BIT)
    {
      cmd.idx = MMC_CMD_APP_CMD;
      cmd.responseType = MMC_RSP_R1;
      cmd.arg = mmc->rca << 16;
      cmd.flags = 0;
      if ((err = mmc->sendCommand(mmc, &cmd, 0)))
      {
        return err;
      }

      cmd.idx = SD_CMD_APP_SET_BUS_WIDTH;
      cmd.responseType = MMC_RSP_R1;
      cmd.arg = 2;
      cmd.flags = 0;
      err = mmc->sendCommand(mmc, &cmd, 0);
      if (err)
      {
        return err;
      }

      mmcSetBusWidth(mmc, 4);
    }

    if (mmc->cardCaps & MMC_MODE_HS)
    {
      mmcSetClock(mmc, 50000000);
    }
    else
    {
      mmcSetClock(mmc, 25000000);
    }
  }

  mmc->blockDev.blockSize = mmc->readBlockLength;
  mmc->blockDev.lba = mmc->capacity / mmc->readBlockLength;

#ifdef MMC_DEBUG
  printf("mmcStartup(): blockDev blocksize = %x\n", mmc->blockDev.blockSize);
  printf("mmcStartup(): blockDev capacity = %x\n", mmc->capacity);
  printf("mmcStartup(): blockDev readBlockLength = %x\n", mmc->readBlockLength);
  printf("mmcStartup(): blockDev blockDev.lba = %x\n", mmc->blockDev.lba);
#endif
  return 0;
}

int mmcSendIfCond(struct mmc *mmc)
{
  struct mmcCommand cmd;
  int err;
  
  cmd.idx = SD_CMD_SEND_IF_COND;
  /* We set the bit if the host supports voltages between 2.7 and 3.6 V */
  cmd.arg = ((mmc->voltages & 0xff8000) != 0) << 8 | 0xaa;
  cmd.responseType = MMC_RSP_R7;
  cmd.flags = 0;

  err = mmc->sendCommand(mmc, &cmd, 0);

  if (err)
  {
    return err;
  }

  if ((cmd.resp[0] & 0xff) != 0xaa)
  {
    return UNUSABLE_ERR;
  }
  else
  {
    mmc->version = SD_VERSION_2;
  }

  return 0;
}

int sdSendOpCond(struct mmc *mmc)
{
  int timeout = 1000;
  int err;
  struct mmcCommand cmd;

  do 
  {
    cmd.idx = MMC_CMD_APP_CMD;
    cmd.responseType = MMC_RSP_R1;
    cmd.arg = 0;
    cmd.flags = 0;

    err = mmc->sendCommand(mmc, &cmd, 0);

    if (err)
    {
      return err;
    }
    cmd.idx = SD_CMD_APP_SEND_OP_COND;
    cmd.responseType = MMC_RSP_R3;

    /*
     * Most cards do not answer if some reserved bits
     * in the ocr are set. However, Some controller
     * can set bit 7 (reserved for low voltages), but
     * how to manage low voltages SD card is not yet
     * specified.
     */
    cmd.arg = mmc->voltages & 0xff8000;

    if (mmc->version == SD_VERSION_2)
    {
      cmd.arg |= OCR_HCS;
    }

    err = mmc->sendCommand(mmc, &cmd, 0);

    if (err)
    {
      return err;
    }

    mdelay32k(1);
  } 
  while ((!(cmd.resp[0] & OCR_BUSY)) && timeout--);

  if (timeout <= 0)
  {
    return UNUSABLE_ERR;
  }

  if (mmc->version != SD_VERSION_2)
  {
    mmc->version = SD_VERSION_1_0;
  }

  mmc->ocr = cmd.resp[0];

  mmc->highCapacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
  mmc->rca = 0;

  return 0;
}

/* Initialize the entire MMC subsystem. Will fail if a card is not detected. */
int mmcMainInit()
{
#ifdef MMC_DEBUG
  printf("mmcMainInit(): Initializing mmc subsystem...\n");
#endif

  //if we don't initialize this then bad things happen on re-runs due to
  //the old value still being in memory, incremented
  // TODO look into this problem later, will probably need complete
  // redesign to fix
  mmcRegisteredNumber = 0; 
  //initialise the host controller, mmc device is now registered
  mmcDevice = mmcInterfaceInit(); 
  
  struct mmc *mmc = mmcDevice;

  // need to perform backend device controller initialization
  int err = 0;
  if ((err = mmc->init(mmc)))
  {
#ifdef MMC_DEBUG
    printf("mmcMainInit(): mmc init failed\n");
#endif
    return err;
  }

  mmcSetBusWidth(mmc, 1);
  mmcSetClock(mmc, 1);

  if ((err = mmcGoIdle(mmc)))
  {
#ifdef MMC_DEBUG
    printf("mmcMainInit(): mmcGoIdle failed\n");
#endif
    return err;
  }

  //test for SD version 2
  if ((err = mmcSendIfCond(mmc)))
  {
#ifdef MMC_DEBUG
    printf("mmcMainInit(): mmcSendIfCond failed\n");
#endif
    return err;
  }

  //try to get SD card's operating condition; ignore mmc cards
  if ((err = sdSendOpCond(mmc)))
  {
#ifdef MMC_DEBUG
    printf("mmcMainInit(): sdSendOpCond failed\n");
#endif
    return err;
  }

  if ((err = mmcStartup(mmc)))
  {
#ifdef MMC_DEBUG
    printf("mmcMainInit(): mmcStartup failed\n");
#endif
    return err;
  }
  return 0; 
}

