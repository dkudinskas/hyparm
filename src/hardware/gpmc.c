#include "gpmc.h"
#include "memoryConstants.h" // for BEAGLE_RAM_START/END
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"

extern GCONTXT * getGuestContext(void);

struct Gpmc * gpmc;

void initGpmc()
{
  gpmc = (struct Gpmc*)mallocBytes(sizeof(struct Gpmc));
  if (gpmc == 0)
  {
    serial_ERROR("Failed to allocate GPMC.");
  }
  else
  {
    memset((void*)gpmc, 0x0, sizeof(struct Gpmc));
#ifdef GPMC_DBG
    serial_putstring("GPMC at 0x");
    serial_putint((u32int)gpmc);
    serial_newline();
#endif
  }
  
  // register default values
  gpmc->gpmcSysConfig = 0x00000010;
  gpmc->gpmcSysStatus = 0x00000001;
  gpmc->gpmcIrqStatus = 0x00000100;
  gpmc->gpmcIrqEnable = 0x00000000; // TODO: verify default values below this point
  gpmc->gpmcTimeoutControl = 0x00000000;
  gpmc->gpmcErrAddress = 0x00000000;
  gpmc->gpmcErrType = 0x00000000;
  gpmc->gpmcConfig = 0x00000000;
  gpmc->gpmcStatus = 0x00000000;
  gpmc->gpmcPrefetchConfig1 = 0x00000000;
  gpmc->gpmcPrefetchConfig2 = 0x00000000;
  gpmc->gpmcPrefetchControl = 0x00000000;
  gpmc->gpmcPrefetchStatus = 0x00000000;
  gpmc->gpmcEccConfig = 0x00000000;
  gpmc->gpmcEccControl = 0x00000000;
  gpmc->gpmcEccSizeConfig = 0x00000000;
  // TODO: add rest
}

/* load function */
u32int loadGpmc(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef SYS_CTRL_MOD_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" access size ");
  serial_putint((u32int)size);
  serial_newline();
#endif

  if (size != WORD)
  {
    // only word access allowed in these modules
    serial_ERROR("Gpmc: invalid access size.");
  }

  u32int regOffset = phyAddr - Q1_L3_GPMC;
  u32int val = 0;
  switch (regOffset)
  {
    case GPMC_REVISION:
      val = GPMC_REVISION_VALUE;
      break;
    case GPMC_SYSCONFIG:
      // TODO
      serial_putstring("WARN reading GPMC_SYSCONFIG ");
      serial_putint(gpmc->gpmcSysConfig);
      serial_newline();
      val = gpmc->gpmcSysConfig;
      break;
    default:
      serial_ERROR("Gpmc: load on invalid register.");
  }
  
  return val;
}


/* top store function */
void storeGpmc(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef GPMC_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" store to pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_putstring(" aSize ");
  serial_putint((u32int)size);
  serial_putstring(" val ");
  serial_putint(value);
  serial_newline();
#endif
  u32int regOffset = phyAddr - Q1_L3_GPMC;
  switch (regOffset)
  {
    case GPMC_SYSCONFIG:
      // TODO
      serial_putstring("WARN writing to GPMC_SYSCONFIG ");
      serial_putint(value);
      serial_newline();
      gpmc->gpmcSysConfig = value;
      break;
    default:
      serial_ERROR("Gpmc: store to invalid register.");
  }
}

