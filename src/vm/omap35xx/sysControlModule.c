#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sysControlModule.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


struct SystemControlModule *sysCtrlModule;


void initSysControlModule()
{
  sysCtrlModule = (struct SystemControlModule *)mallocBytes(sizeof(struct SystemControlModule));
  if (sysCtrlModule == 0)
  {
    DIE_NOW(0, "Failed to allocate system control module.");
  }
  else
  {
    memset(sysCtrlModule, 0x0, sizeof(struct SystemControlModule));
    DEBUG(VP_OMAP_35XX_SCM, "Initializing system control module at %p" EOL, sysCtrlModule);
  }

  // register default values
  // SYS_CTRL_MOD_INTERFACE      0x48002000 base, 36 bytes length
  // TODO
  // SYS_CTRL_MOD_PADCONFS       0x48002030 base, 564 bytes length
  // TODO
  // SYS_CTRL_MOD_GENERAL        0x48002270 base, 767 bytes length
  sysCtrlModule->ctrlPadConfOff = 0x00000000;
  sysCtrlModule->ctrlDevConf0 = 0x05000000;
  sysCtrlModule->ctrlMemDftrw0 = 0x00000000;
  sysCtrlModule->ctrlMemDftrw1 = 0x00000000;
  sysCtrlModule->ctrlMsuspendMux0 = 0x00000000;
  sysCtrlModule->ctrlMsuspendMux1 = 0x00000000;
  sysCtrlModule->ctrlMsuspendMux2 = 0x00248000;
  sysCtrlModule->ctrlMsuspendMux3 = 0x00000000;
  sysCtrlModule->ctrlMsuspendMux4 = 0x00000000;
  sysCtrlModule->ctrlMsuspendMux5 = 0x00000000;
  sysCtrlModule->ctrlSecCtrl = 0x00001881;
  sysCtrlModule->ctrlDevConf1 = 0x00000000;
  sysCtrlModule->ctrlCsiRxfe = 0x00000000;
  sysCtrlModule->ctrlSecStatus = 0x00000000;
  sysCtrlModule->ctrlSecErrStatus = 0x00000000;
  sysCtrlModule->ctrlSecErrStatusDbg = 0x00000000;
  sysCtrlModule->ctrlStatus = 0x0000030f;
  sysCtrlModule->ctrlGpStatus = 0x00000000;
  sysCtrlModule->ctrlRpubKeyH0 = 0x00000000;
  sysCtrlModule->ctrlRpubKeyH1 = 0x00000000;
  sysCtrlModule->ctrlRpubKeyH2 = 0x00000000;
  sysCtrlModule->ctrlRpubKeyH3 = 0x00000000;
  sysCtrlModule->ctrlRpubKeyH4 = 0x00000000;
   // not accessible on the beagle?...
  /*
  sysCtrlModule->ctrlRandKey0 = 0x;
  sysCtrlModule->ctrlRandKey1 = 0x;
  sysCtrlModule->ctrlRandKey2 = 0x;
  sysCtrlModule->ctrlRandKey3 = 0x;
  sysCtrlModule->ctrlCustKey0 = 0x;
  sysCtrlModule->ctrlCustKey1 = 0x;
  sysCtrlModule->ctrlCustKey2 = 0x;
  sysCtrlModule->ctrlCustKey3 = 0x;
  */
   // .. up to here
  sysCtrlModule->ctrlUsbConf0 = 0x00000000;
  sysCtrlModule->ctrlUsbConf1 = 0x00000000;
  sysCtrlModule->ctrlFuseOpp1Vdd1 = 0x0099bc84;
  sysCtrlModule->ctrlFuseOpp2Vdd1 = 0x009a88c1;
  sysCtrlModule->ctrlFuseOpp3Vdd1 = 0x00aab48a;
  sysCtrlModule->ctrlFuseOpp4Vdd1 = 0x00aba2e6;
  sysCtrlModule->ctrlFuseOpp5Vdd1 = 0x00ab90d3;
  sysCtrlModule->ctrlFuseOpp1Vdd2 = 0x0099be86;
  sysCtrlModule->ctrlFuseOpp2Vdd2 = 0x009a89c4;
  sysCtrlModule->ctrlFuseOpp3Vdd2 = 0x00aac695;
  sysCtrlModule->ctrlFuseSr = 0x00000a0f;
  sysCtrlModule->ctrlCek0 = 0x00000000;
  sysCtrlModule->ctrlCek1 = 0x00000000;
  sysCtrlModule->ctrlCek2 = 0x00000000;
  sysCtrlModule->ctrlCek3 = 0x00000000;
  sysCtrlModule->ctrlMsv0 = 0x00000000;
  sysCtrlModule->ctrlCekBch0 = 0x00000000;
  sysCtrlModule->ctrlCekBch1 = 0x00000000;
  sysCtrlModule->ctrlCekBch2 = 0x00000000;
  sysCtrlModule->ctrlCekBch3 = 0x00000000;
  sysCtrlModule->ctrlCekBch4 = 0x00000000;
  sysCtrlModule->ctrlMsvBch0 = 0x00000000;
  sysCtrlModule->ctrlMsvBch1 = 0x00000000;
  sysCtrlModule->ctrlSwrv0 = 0x02000000;
  sysCtrlModule->ctrlSwrv1 = 0x00000000;
  sysCtrlModule->ctrlSwrv2 = 0x00008000;
  sysCtrlModule->ctrlSwrv3 = 0x00080100;
  sysCtrlModule->ctrlSwrv4 = 0x00200000;
  sysCtrlModule->ctrlIva2Bootaddr = 0x00000000;
  sysCtrlModule->ctrlIva2Bootmod = 0x00000000;
  sysCtrlModule->ctrlDebobs0 = 0x00000000;
  sysCtrlModule->ctrlDebobs1 = 0x00000000;
  sysCtrlModule->ctrlDebobs2 = 0x00000000;
  sysCtrlModule->ctrlDebobs3 = 0x00000000;
  sysCtrlModule->ctrlDebobs4 = 0x00000000;
  sysCtrlModule->ctrlDebobs5 = 0x00000000;
  sysCtrlModule->ctrlDebobs6 = 0x00000000;
  sysCtrlModule->ctrlDebobs7 = 0x00000000;
  sysCtrlModule->ctrlDebobs8 = 0x00000000;
  sysCtrlModule->ctrlProgIO0 = 0x00007fc0;
  sysCtrlModule->ctrlProgIO1 = 0x0002aaaa;
  sysCtrlModule->ctrlWkupCtrl = 0x00000000; // ??? @ off 0x00000A5C
  sysCtrlModule->ctrlDssDpllSpreading = 0x00000040;
  sysCtrlModule->ctrlCoreDpllSpreading = 0x00000040;
  sysCtrlModule->ctrlPerDpllSpreading = 0x00000040;
  sysCtrlModule->ctrlUsbhostDpllSpreading = 0x00000040;
  sysCtrlModule->ctrlSecSdrcSharing = 0x00002700;
  sysCtrlModule->ctrlSecSdrcMcfg0 = 0x00300000;
  sysCtrlModule->ctrlSecSdrcMcfg1 = 0x00300000;
  sysCtrlModule->ctrlModemFwConfLock = 0x00000000;
  sysCtrlModule->ctrlModemMemResConf = 0x00000000;
  sysCtrlModule->ctrlModemGpmcDtFwReqInfo = 0x0000ffff;
  sysCtrlModule->ctrlModemGpmcDtFwRd = 0x0000ffff;
  sysCtrlModule->ctrlModemGpmcDtFwWr = 0x0000ffff;
  sysCtrlModule->ctrlModemGpmcBootCode = 0x00000000;
  sysCtrlModule->ctrlModemSmsRgAtt1 = 0xffffffff;
  sysCtrlModule->ctrlModemSmsRgRdPerm1 = 0x0000ffff;
  sysCtrlModule->ctrlModemSmsRgWrPerm1 = 0x0000ffff;
  sysCtrlModule->ctrlModemD2dFwDbgMode = 0x00000000;
  sysCtrlModule->ctrlDpfOcmRamFwAddrMatch = 0x00000000;
  sysCtrlModule->ctrlDpfOcmRamFwReqinfo = 0x00000000;
  sysCtrlModule->ctrlDpfOcmRamFwWr = 0x00000000;
  sysCtrlModule->ctrlDpfReg4GpmcFwAddrMatch = 0x00000000;
  sysCtrlModule->ctrlDpfReg4GpmcFwReqinfo = 0x00000000;
  sysCtrlModule->ctrlDpfReg4GpmcFwWr = 0x00000000;
  sysCtrlModule->ctrlDpfReg1Iva2FwAddrMatch = 0x00000000;
  sysCtrlModule->ctrlDpfReg1Iva2FwReqinfo = 0x00000000;
  sysCtrlModule->ctrlDpfReg1Iva2FwWr = 0x00000000;
  sysCtrlModule->ctrlApeFwDefSecLock = 0x00000000;
  sysCtrlModule->ctrlOcmRomSecDbg = 0x00000000;
  sysCtrlModule->ctrlExtSecCtrl = 0x00000002;
  sysCtrlModule->ctrlPbiasLite = 0x00000b87;
  sysCtrlModule->ctrlCsi = 0x03200000;
  sysCtrlModule->ctrlDpfMad2dFwAddrMatch = 0x00000000;
  sysCtrlModule->ctrlDpfMad2dFwReqinfo = 0x00000000;
  sysCtrlModule->ctrlDpfMad2dFwWr = 0x00000000;
  sysCtrlModule->ctrlIdCode = 0x3b7ae02f; // offs 0x00307F94, phys 0x4830A204 out of range
  // SYS_CTRL_MOD_MEM_WKUP       0x48002600 base, 1024 bytes length
  // this is just a memory blob of 1k
  // SYS_CTRL_MOD_PADCONFS_WKUP  0x48002A00 base, 80 bytes length
  // TODO
  // SYS_CTRL_MOD_GENERAL_WKUP   0x48002A60 base, 31 bytes length
  // TODO
}

/* load function */
u32int loadSysCtrlModule(device *dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  DEBUG(VP_OMAP_35XX_SCM, "%s load from pAddr: %#.8x, vAddr %#.8x, aSize %x" EOL, dev->deviceName,
      phyAddr, address, (u32int)size);

  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(gc, "SysControlModule: invalid access size.");
  }

  u32int val = 0;

  if ((phyAddr >= SYS_CTRL_MOD_INTERFACE) && (phyAddr < (SYS_CTRL_MOD_INTERFACE + 36)))
  {
    val = loadInterfaceScm(dev, address, phyAddr);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_PADCONFS) && (phyAddr < (SYS_CTRL_MOD_PADCONFS + 564)))
  {
    val = loadPadconfsScm(dev, address, phyAddr);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_GENERAL) && (phyAddr < (SYS_CTRL_MOD_GENERAL + 767)))
  {
    val = loadGeneralScm(dev, address, phyAddr);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_MEM_WKUP) && (phyAddr < (SYS_CTRL_MOD_MEM_WKUP + 1024)))
  {
    val = loadMemWkupScm(dev, address, phyAddr);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_PADCONFS_WKUP) && (phyAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + 80)))
  {
    val = loadPadconfsWkupScm(dev, address, phyAddr);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_GENERAL_WKUP) && (phyAddr < (SYS_CTRL_MOD_GENERAL_WKUP + 31)))
  {
    val = loadGeneralWkupScm(dev, address, phyAddr);
  }
  else
  {
    DIE_NOW(gc, "SysControlModule: invalid base module.");
  }

  return val;
}

u32int loadInterfaceScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(0, "loadInterfaceScm unimplemented.");
  return 0;
}


u32int loadPadconfsScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(0, "loadPadconfsScm unimplemented.");
  return 0;
}


u32int loadGeneralScm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - SYS_CTRL_MOD_GENERAL;
  switch (reg)
  {
    case CONTROL_DEVCONF0:
      val = sysCtrlModule->ctrlDevConf0;
      break;
    case CONTROL_DEVCONF1:
      val = sysCtrlModule->ctrlDevConf1;
      break;
    default:
      printf("loadGeneralScm: unimplemented reg addr %#.8x" EOL, phyAddr);
      DIE_NOW(0, "loadGeneralScm loading non existing/unimplemented register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_SCM, "loadGeneralScm reg %x value %#.8x" EOL, reg, val);
  return val;
}


u32int loadMemWkupScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(0, "loadMemWkupScm unimplemented.");
  return 0;
}


u32int loadPadconfsWkupScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(0, "loadPadconfsWkupScm unimplemented.");
  return 0;
}


u32int loadGeneralWkupScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(0, "loadGeneralWkupScm unimplemented.");
  return 0;
}

/* top store function */
void storeSysCtrlModule(device *dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

  DEBUG(VP_OMAP_35XX_SCM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %x, value %#.8x" EOL,
      dev->deviceName, phyAddr, address, (u32int)size, value);

  if ((phyAddr >= SYS_CTRL_MOD_INTERFACE) && (phyAddr < (SYS_CTRL_MOD_INTERFACE + 36)))
  {
    storeInterfaceScm(dev, address, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_PADCONFS) && (phyAddr < (SYS_CTRL_MOD_PADCONFS + 564)))
  {
    storePadconfsScm(dev, address, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_GENERAL) && (phyAddr < (SYS_CTRL_MOD_GENERAL + 767)))
  {
    storeGeneralScm(dev, address, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_MEM_WKUP) && (phyAddr < (SYS_CTRL_MOD_MEM_WKUP + 1024)))
  {
    storeMemWkupScm(dev, address, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_PADCONFS_WKUP) && (phyAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + 80)))
  {
    storePadconfsWkupScm(dev, address, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_GENERAL_WKUP) && (phyAddr < (SYS_CTRL_MOD_GENERAL_WKUP + 31)))
  {
    storeGeneralWkupScm(dev, address, phyAddr, value);
  }
  else
  {
    printf("%s store to pAddr: %#.8x, vAddr %#.8x, aSize %x, value %#.8x" EOL,
        dev->deviceName, phyAddr, address, (u32int)size, value);
    DIE_NOW(gc, "Invalid base module.");
  }
}

void storeInterfaceScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(0, "storeInterfaceScm unimplemented.");
}

void storePadconfsScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(0, "storePadconfsScm unimplemented.");
}

void storeGeneralScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(0, "storeGeneralScm unimplemented.");
}

void storeMemWkupScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(0, "storeMemWkupScm unimplemented.");
}

void storePadconfsWkupScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(0, "storePadconfsWkupScm unimplemented.");
}

void storeGeneralWkupScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(0, "storeGeneralWkupScm unimplemented.");
}
