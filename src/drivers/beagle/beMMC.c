#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/be32kTimer.h"
#include "drivers/beagle/beMMC.h"

#include "common/debug.h"


struct mmc mmchs;


static inline u32int readWord(u32int addr);
static inline void writeWord(u32int addr, u32int value);


static inline u32int readWord(u32int addr)
{
  return *(volatile u32int *)addr;
}

static inline void writeWord(u32int addr, u32int value)
{
  *(volatile u32int *)addr = value;
}

/* Host controller initialization needed before any MMC ops */
int mmcInit(struct mmc *mmc)
{
  mmc->base = MMCHS_BASE;

  mmcEnableClocks();
  mmcSoftReset(mmc);
  mmcSetVoltageCapa(mmc);
  mmcWakeupConfig(mmc);

  struct mmc *dev = mmc;
  //disable bus power
  writeWord(dev->base + MMCHS_HCTL, (HCTL_DTW_1BIT & (~HCTL_SDBP)) | HCTL_SDVS_30V);

  // set voltage capabilities for this module
  u32int reg = readWord(dev->base + MMCHS_CAPA);
  reg |= CAPA_VS30 | CAPA_VS18;
  writeWord(dev->base + MMCHS_CAPA, reg);

  reg = readWord(dev->base + MMCHS_CON) & RESERVED_MASK;
  writeWord(dev->base | MMCHS_CON,reg); //only default values needed here

  u32int dsor = 240;
  reg = readWord(dev->base + MMCHS_SYSCTL);
  reg &= ~(SYSCTL_ICE | SYSCTL_DTO_MASK | SYSCTL_CEN);
  writeWord(dev->base + MMCHS_SYSCTL, (reg & ~(SYSCTL_ICE | SYSCTL_CEN)) | SYSCTL_DTO_15THDTO);

  reg = readWord(dev->base + MMCHS_SYSCTL) & ~(SYSCTL_ICE | CLKD_1023_M);
  writeWord(dev->base + MMCHS_SYSCTL, (dsor << CLKD_OFFSET) | SYSCTL_ICE);

  DEBUG(PP_OMAP_35XX_MMCHS, "mmcInit: waiting for Internal Clock Stable (ICS)...");
  while (!(readWord(dev->base + MMCHS_SYSCTL) & SYSCTL_ICS))
  {
    // do nothing
  }
  DEBUG(PP_OMAP_35XX_MMCHS, "done" EOL);

  // enable internal clock
  writeWord(dev->base + MMCHS_SYSCTL, readWord(dev->base | MMCHS_SYSCTL) | SYSCTL_CEN);

  //re-enable bus power
  writeWord(dev->base + MMCHS_HCTL, readWord(dev->base + MMCHS_HCTL) | HCTL_SDBP);

  // enable interrupts: bad access, card error, data/bit error, data CRC error, data timeout,
  // command index error, command end bit error, command CRC error, command timeout,
  // buffer read ready, buffer write ready, transter complete, command completed
  writeWord(dev->base + MMCHS_IE,
              (IE_BADA | IE_CERR | IE_DEB | IE_DCRC | IE_DTO | IE_CIE |
              IE_CEB | IE_CCRC | IE_CTO | IE_BRR | IE_BWR | IE_TC | IE_CC));

  // perform the initializatino stream command
  writeWord(dev->base + MMCHS_CON, readWord(dev->base + MMCHS_CON) | INIT_INITSTREAM);

  writeWord(dev->base | MMCHS_CMD, CMD0);

  // the init stream must be maintained for at least 80 cycles
  mdelay32k(50); // 50ms delay

  DEBUG(PP_OMAP_35XX_MMCHS, "mmcInit: waiting for CC...");
  while (!(readWord(dev->base + MMCHS_STAT) | STAT_CC))
  {
    // do nothing
  }
  DEBUG(PP_OMAP_35XX_MMCHS, "done" EOL);

  // clear command complete status bit
  writeWord(dev->base + MMCHS_STAT, STAT_CC);
  writeWord(dev->base + MMCHS_CMD, CMD0);
  DEBUG(PP_OMAP_35XX_MMCHS, "mmcInit: waiting for CC2...");
  while (!(readWord(dev->base + MMCHS_STAT) | STAT_CC))
  {
    // do nothing
  }
  DEBUG(PP_OMAP_35XX_MMCHS, "done" EOL);

  // disable the init steam (should have lasted longer than 80 clock cycles)
  writeWord(dev->base + MMCHS_CON, readWord(dev->base + MMCHS_CON) & ~INIT_INITSTREAM);

  return 0;
}

void mmcEnableClocks()
{
  DEBUG(PP_OMAP_35XX_MMCHS, "MMCHS: Enabling clocks...");
  //enable functional clocks
  u32int reg = clkManRegReadBE(CORE_CM, CM_FCLKEN1_CORE);
  reg |= EN_MMC1 | EN_MMC2 | EN_MMC3;
  clkManRegWriteBE(CORE_CM, CM_FCLKEN1_CORE, reg);

  //enable the interface clocks
  reg = clkManRegReadBE(CORE_CM, CM_ICLKEN1_CORE);
  reg |= EN_MMC1 | EN_MMC2 | EN_MMC3;
  clkManRegWriteBE(CORE_CM, CM_ICLKEN1_CORE, reg);
  DEBUG(PP_OMAP_35XX_MMCHS, "done" EOL);
}

void mmcSoftReset(struct mmc *dev)
{
  u32int reg = readWord(dev->base + MMCHS_SYSCONFIG);
  writeWord(dev->base + MMCHS_SYSCONFIG, reg | SYSCONFIG_SOFTRESET);

  DEBUG(PP_OMAP_35XX_MMCHS, "MMCHS: Waiting for reset... ");
  //check if reset is finished
  while (!(readWord(dev->base + MMCHS_SYSSTATUS) & SYSSTATUS_RESETDONE))
  {
    // do nothing
  }
  DEBUG(PP_OMAP_35XX_MMCHS, "done" EOL);

  reg = readWord(dev->base + MMCHS_SYSCTL);
  writeWord(dev->base + MMCHS_SYSCTL, reg | SYSCTL_SRA);

  DEBUG(PP_OMAP_35XX_MMCHS, "MMCHS: Waiting for software reset for all... ");
  while (readWord(dev->base + MMCHS_SYSCTL) & SYSCTL_SRA)
  {
    // do nothing
  }
  DEBUG(PP_OMAP_35XX_MMCHS, "done" EOL);
}

void mmcSetVoltageCapa(struct mmc *dev)
{
  //for all MMCHS modules, 1.8V must be set
  //3.0V for MMCHS1 must be set and disabled for others
  u32int reg = readWord(MMCHS1 + MMCHS_CAPA);
  writeWord(MMCHS1 + MMCHS_CAPA, reg | CAPA_VS18 | CAPA_VS30);
  //don't set current capabilities for now, not much info
}

void mmcWakeupConfig(struct mmc *dev)
{
  u32int reg = readWord(dev->base + MMCHS_SYSCONFIG);
  // disable wakeup for now
  writeWord(dev->base + MMCHS_SYSCONFIG, reg & ~SYSCONFIG_ENAWAKEUP);

  reg = readWord(dev->base + MMCHS_HCTL);
  writeWord(dev->base + MMCHS_HCTL, reg & ~HCTL_IWE);

  // mask interrupts for SDIO cards
  reg = readWord(dev->base + MMCHS_IE);
  writeWord(dev->base + MMCHS_IE, reg & ~IE_CIRQ_ENABLE);

  //TODO review the interrupts in IE, will want to enable more eventually
}

/* Send a command */
int mmcSendCommand(struct mmc *dev, struct mmcCommand *cmd, struct mmcData *data)
{
  u32int flags;
  DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: waiting for PSTATE_DATI clear..." EOL);
  while (readWord(dev->base + MMCHS_PSTATE) & PSTATE_DATI)
  {
    // do nothing
  }

  DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: waiting for PSTATE_CMDI clear..." EOL);
  while (readWord(dev->base + MMCHS_PSTATE) & PSTATE_CMDI)
  {
    // do nothing
  }

  writeWord(dev->base + MMCHS_STAT, 0xFFFFFFFF);
  DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: waiting for stat to clear" EOL);
  while (readWord(dev->base + MMCHS_STAT))
  {
    // do nothing
  }

  /*
   * CMDREG
   * CMDIDX[13:8] : Command index
   * DATAPRNT[5]  : Data Present Select
   * ENCMDIDX[4]  : Command Index Check Enable
   * ENCMDCRC[3]  : Command CRC Check Enable
   * RSPTYP[1:0]
   *  00 = No Response
   *  01 = Length 136
   *  10 = Length 48
   *  11 = Length 48 Check busy after response
   */
  if (cmd->idx == SD_CMD_APP_SEND_SCR)
  {
    mdelay32k(50); // 50ms delay
  }

  if (!(cmd->responseType & MMC_RSP_PRESENT))
  {
    flags = 0;
  }
  else if (cmd->responseType & MMC_RSP_136)
  {
    flags = RSP_TYPE_LGHT136 | CICE_NOCHECK;
  }
  else if (cmd->responseType & MMC_RSP_BUSY)
  {
    flags = RSP_TYPE_LGHT48B;
  }
  else
  {
    flags = RSP_TYPE_LGHT48;
  }

  // enable default flags
  flags = flags | (CMD_TYPE_NORMAL | CICE_NOCHECK | CCCE_NOCHECK | MSBS_SGLEBLK |
                   ACEN_DISABLE | BCE_DISABLE | DE_DISABLE);

  if (cmd->responseType & MMC_RSP_CRC)
  {
    flags |= CCCE_CHECK;
  }
  if (cmd->responseType & MMC_RSP_OPCODE)
  {
    flags |= CICE_CHECK;
  }

  if (data)
  {
    DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: Data command: ");
    /* Set block & write flags if we're doing a data op */
    if ((cmd->idx == MMC_CMD_READ_MULTIPLE_BLOCK) ||
        (cmd->idx == MMC_CMD_WRITE_MULTIPLE_BLOCK))
    {
      DEBUG(PP_OMAP_35XX_MMCHS, "r/w multiple block" EOL);
      flags |= (MSBS_MULTIBLK | BCE_ENABLE);
      data->blocksize = 512;
      writeWord(dev->base + MMCHS_BLK, (data->blocksize | (data->blocks << 16)));
    }
    else
    {
      DEBUG(PP_OMAP_35XX_MMCHS, "single block" EOL);
      writeWord(dev->base + MMCHS_BLK, data->blocksize | NBLK_STPCNT);
    }

    if (data->flags & MMC_DATA_READ)
    {
      flags |= (DP_DATA | DDIR_READ);
    }
    else
    {
      flags |= (DP_DATA | DDIR_WRITE);
    }
  }

  writeWord(dev->base + MMCHS_ARG, cmd->arg);
  writeWord(dev->base + MMCHS_CMD, (cmd->idx << 24) | flags);

  u32int stat;
  int timeout = 0;
  do
  {
    if (timeout > 1000000)
    {
      printf("stat timeout" EOL);
      return 0;
    }
    timeout++;
    stat = readWord(dev->base + MMCHS_STAT);
  }
  while (!stat);

  //treat timeouts same as error for now
  if (stat & STAT_CTO)
  {
    DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: stat timeout error" EOL);
    return -1;
  }
  else if (stat & STAT_ERRI)
  {
    DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: stat error interrupt" EOL);
    return -1;
  }

  // is command completed?
  if (stat & STAT_CC)
  {
    // need to ack/clear CC stat bit
    writeWord(dev->base + MMCHS_STAT, STAT_CC);

    if (cmd->responseType & MMC_RSP_PRESENT)
    {
      if (cmd->responseType & MMC_RSP_136)
      {
        /* response type 2 */
        cmd->resp[3] = readWord(dev->base + MMCHS_RSP10);
        cmd->resp[2] = readWord(dev->base + MMCHS_RSP32);
        cmd->resp[1] = readWord(dev->base + MMCHS_RSP54);
        cmd->resp[0] = readWord(dev->base + MMCHS_RSP76);
      }
      else
      {
        cmd->resp[0] = readWord(dev->base + MMCHS_RSP10);
      }
    }
  } // if command completed ends

  if (data && (data->flags & MMC_DATA_READ))
  {
    DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: data read command, dest = %#x size %#x" EOL,
            data->dest, data->blocksize * data->blocks);
    // doing a read op, start the read process
    mmcRead(dev, data->dest, data->blocksize * data->blocks);
  }
  else if (data && (data->flags & MMC_DATA_WRITE))
  {
    DEBUG(PP_OMAP_35XX_MMCHS, "mmcSendCommand: data write command, src = %#x size %#x" EOL,
            data->src, data->blocksize * data->blocks);
    mmcWrite(dev, data->src, data->blocksize * data->blocks);
  }
  return 0;
}


int mmcRead(struct mmc* dev, char* buf, u32int size)
{
  u32int* out = (u32int*)buf;
  u32int stat = 0;
  u32int count = (size > MMCSD_SECTOR_SIZE) ? MMCSD_SECTOR_SIZE : size;
  // transferring in words
  count /= 4;

  /* Start polled read */
  while (size)
  {
    // wait for stat
    do
    {
      stat = readWord(dev->base + MMCHS_STAT);
    }
    while (!stat);

    if (stat & STAT_ERRI)
    {
      return 1;
    }

    if (stat & STAT_BRR)
    {
      //HS read buffer is full and ready to read
      u32int index;

      writeWord(dev->base + MMCHS_STAT, readWord(dev->base + MMCHS_STAT) | STAT_BRR);

      for (index = 0; index < count; index++)
      {
        *out = readWord(dev->base + MMCHS_DATA);
        out++;
      }
      size -= (count * 4);
    }

    // no idea why the uboot driver checks for buffer-write-ready in a read op
    if (stat & STAT_BWR)
    {
      writeWord(dev->base + MMCHS_STAT, readWord(dev->base + MMCHS_STAT) | STAT_BWR);
    }

    if (stat & STAT_TC)
    {
      // read transfer complete
      writeWord(dev->base + MMCHS_STAT, readWord(dev->base + MMCHS_STAT) | STAT_TC);
      break;
    }
  }

  return 0;
}


int mmcWrite(struct mmc *dev, const char *buf, u32int size)
{
  u32int *input = (u32int *)buf;
  u32int stat, count;

  /* start polled write */
  if (size > MMCSD_SECTOR_SIZE)
  {
    count = MMCSD_SECTOR_SIZE;
  }
  else
  {
    count = size;
  }

  count /= 4;

  while (size)
  {
    do
    {
      stat = readWord(dev->base + MMCHS_STAT);
    }
    while (!stat);

    if (stat & STAT_ERRI)
    {
      printf("mmcWrite: STAT error = %#.8x" EOL, stat);
      return 1;
    }

    if (stat & STAT_BWR)
    {
      // HS write buffer has at least one block empty
      u32int index;

      writeWord(dev->base + MMCHS_STAT, readWord(dev->base + MMCHS_STAT) | STAT_BWR);

      for (index = 0; index < count; index++)
      {
        writeWord(dev->base + MMCHS_DATA, *input);
        input++;
      }
      size -= count * 4;
    }

    if (stat & STAT_TC)
    {
      writeWord(dev->base + MMCHS_STAT, readWord(dev->base + MMCHS_STAT) | STAT_TC);
      break;
    }
  } //while size

  return 0;
}

/* Apply the bus width and clock settings to the controller */
void mmcSetIOS(struct mmc *mmc)
{
  u32int divisor = 0;

  if (mmc->busWidth == 8)
  {
    writeWord(mmc->base + MMCHS_CON, readWord(mmc->base + MMCHS_CON) | CON_DW8);
  }
  else if (mmc->busWidth == 4)
  {
    writeWord(mmc->base | MMCHS_CON, readWord(mmc->base | MMCHS_CON) & ~CON_DW8);
    writeWord(mmc->base | MMCHS_HCTL, readWord(mmc->base | MMCHS_HCTL) | HCTL_DTW_4);
  }
  else
  {
    writeWord(mmc->base | MMCHS_CON, readWord(mmc->base | MMCHS_CON) & ~CON_DW8);
    writeWord(mmc->base | MMCHS_HCTL, readWord(mmc->base | MMCHS_HCTL) & ~HCTL_DTW_4);
  }

  //configure clock for max 96mhz
  if (mmc->clock != 0)
  {
    divisor = MMC_CLOCK_REFERENCE * 1000000 / mmc->clock;
    if ((MMC_CLOCK_REFERENCE * 1000000 ) / divisor > mmc->clock)
    {
      divisor++;
    }
  }

  u32int reg = readWord(mmc->base + MMCHS_SYSCTL);
  reg &= ~(SYSCTL_DTO_MASK | SYSCTL_ICE | SYSCTL_CEN); //set ICE & CEN to 0
  writeWord(mmc->base + MMCHS_SYSCTL, reg | SYSCTL_DTO_15THDTO);

  reg = readWord(mmc->base + MMCHS_SYSCTL);
  reg &= ~(SYSCTL_ICE | CLKD_1023_M);
  writeWord(mmc->base + MMCHS_SYSCTL, reg | SYSCTL_ICE | (divisor << CLKD_OFFSET));

  DEBUG(PP_OMAP_35XX_MMCHS, "setIOS: waiting for ICS stabilise..." EOL);
  while ((readWord(mmc->base + MMCHS_SYSCTL) & SYSCTL_ICS) == 0)
  {
    // do nothing
  }

  writeWord(mmc->base + MMCHS_SYSCTL, readWord(mmc->base + MMCHS_SYSCTL) | SYSCTL_CEN);

  DEBUG(PP_OMAP_35XX_MMCHS, "mmcSetIOS: done." EOL);
}


struct mmc* mmcInterfaceInit()
{
  DEBUG(PP_OMAP_35XX_MMCHS, "mmcInterfaceInit(): HS interface init...");

  struct mmc *mmc = &mmchs;

  mmc->sendCommand = mmcSendCommand;
  mmc->setIOS = mmcSetIOS;
  mmc->init = mmcInit;

  mmc->frequencyMin = 400000;
  mmc->frequencyMax = 52000000;

  mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
  mmc->hostCaps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;

  DEBUG(PP_OMAP_35XX_MMCHS, "done" EOL);

  mmcRegister(mmc);

  return mmc;
}

