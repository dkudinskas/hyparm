#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/sysControlModule.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END


struct SystemControlModule *sysCtrlModule;


void initSysControlModule()
{
  sysCtrlModule = (struct SystemControlModule *)calloc(1, sizeof(struct SystemControlModule));
  if (sysCtrlModule == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate system control module.");
  }

  DEBUG(VP_OMAP_35XX_SCM, "Initializing system control module at %p" EOL, sysCtrlModule);

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
  sysCtrlModule->ctrlPadConfSdrcD0   = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD2   = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD4   = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD6   = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD8   = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD10  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD12  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD14  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD16  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD18  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD20  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD22  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD24  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD26  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD28  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcD30  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcClk  = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcCke1 = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcDqs1 = 0x01000100;
  sysCtrlModule->ctrlPadConfSdrcDqs3 = 0x00000100;

  sysCtrlModule->ctrlPadConfGpmcA2    = 0;
  sysCtrlModule->ctrlPadConfGpmcA4    = 0;
  sysCtrlModule->ctrlPadConfGpmcA6    = 0;
  sysCtrlModule->ctrlPadConfGpmcA8    = 0;
  sysCtrlModule->ctrlPadConfGpmcA10   = 0x01000000;
  sysCtrlModule->ctrlPadConfGpmcD1    = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcD3    = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcD5    = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcD7    = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcD9    = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcD11   = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcD13   = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcD15   = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcNcs1  = 0x00180018;
  sysCtrlModule->ctrlPadConfGpmcNcs3  = 0x00180018;
  sysCtrlModule->ctrlPadConfGpmcNcs5  = 0x01010000;
  sysCtrlModule->ctrlPadConfGpmcNcs7  = 0x00000119;
  sysCtrlModule->ctrlPadConfGpmcAle   = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcNwe   = 0x00000100;
  sysCtrlModule->ctrlPadConfGpmcNbe1  = 0x01000100;
  sysCtrlModule->ctrlPadConfGpmcWait0 = 0x01180118;
  sysCtrlModule->ctrlPadConfGpmcWait2 = 0x01180118;

  sysCtrlModule->ctrlPadConfDssPclk   = 0;
  sysCtrlModule->ctrlPadConfDssVsync  = 0;
  sysCtrlModule->ctrlPadConfDssData0  = 0;
  sysCtrlModule->ctrlPadConfDssData2  = 0;
  sysCtrlModule->ctrlPadConfDssData4  = 0;
  sysCtrlModule->ctrlPadConfDssData6  = 0;
  sysCtrlModule->ctrlPadConfDssData8  = 0;
  sysCtrlModule->ctrlPadConfDssData10 = 0;
  sysCtrlModule->ctrlPadConfDssData12 = 0;
  sysCtrlModule->ctrlPadConfDssData14 = 0;
  sysCtrlModule->ctrlPadConfDssData16 = 0;
  sysCtrlModule->ctrlPadConfDssData18 = 0;
  sysCtrlModule->ctrlPadConfDssData20 = 0;
  sysCtrlModule->ctrlPadConfDssData22 = 0;

  sysCtrlModule->ctrlPadConfCamHs    = 0x01180118;
  sysCtrlModule->ctrlPadConfCamXclka = 0x01180000;
  sysCtrlModule->ctrlPadConfCamFld   = 0x01000004;
  sysCtrlModule->ctrlPadConfCamD1    = 0x01000100;
  sysCtrlModule->ctrlPadConfCamD3    = 0x01000100;
  sysCtrlModule->ctrlPadConfCamD5    = 0x01000100;
  sysCtrlModule->ctrlPadConfCamD7    = 0x01000100;
  sysCtrlModule->ctrlPadConfCamD9    = 0x01000100;
  sysCtrlModule->ctrlPadConfCamD11   = 0x00000100;
  sysCtrlModule->ctrlPadConfCamWen   = 0x00000104;

  sysCtrlModule->ctrlPadConfCsi2Dx0  = 0x01000100;
  sysCtrlModule->ctrlPadConfCsi2Dx1  = 0x01000100;

  sysCtrlModule->ctrlPadConfUart3RtsSd  = 0x01000000;
  sysCtrlModule->ctrlPadConfUart3TxIrtx = 0x01000000;

  sysCtrlModule->ctrlPadConfHsusb0Stp   = 0x01000018;
  sysCtrlModule->ctrlPadConfHsusb0Nxt   = 0x01000100;
  sysCtrlModule->ctrlPadConfHsusb0Data1 = 0x01000100;
  sysCtrlModule->ctrlPadConfHsusb0Data3 = 0x01000100;
  sysCtrlModule->ctrlPadConfHsusb0Data5 = 0x01000100;
  sysCtrlModule->ctrlPadConfHsusb0Data7 = 0x01180100;

  sysCtrlModule->ctrlPadConfI2c1Sda = 0x011c0118;
  sysCtrlModule->ctrlPadConfI2c2Sda = 0x0118011c;
  sysCtrlModule->ctrlPadConfI2c3Sda = 0x001c0118;

  sysCtrlModule->ctrlPadConfEtkClk = 0x0013001b;
  sysCtrlModule->ctrlPadConfEtkD0  = 0x01130113;
  sysCtrlModule->ctrlPadConfEtkD2  = 0x01130113;
  sysCtrlModule->ctrlPadConfEtkD4  = 0x01130113;
  sysCtrlModule->ctrlPadConfEtkD6  = 0x01130113;
  sysCtrlModule->ctrlPadConfEtkD8  = 0x01130113;
  sysCtrlModule->ctrlPadConfEtkD10 = 0x00130013;
  sysCtrlModule->ctrlPadConfEtkD12 = 0x01130113;
  sysCtrlModule->ctrlPadConfEtkD14 = 0x01130113;
  // SYS_CTRL_MOD_GENERAL_WKUP   0x48002A60 base, 31 bytes length
  // TODO
}

/* load function */
u32int loadSysCtrlModule(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  DEBUG(VP_OMAP_35XX_SCM, "%s load from pAddr: %#.8x, vAddr %#.8x, aSize %x" EOL, dev->deviceName,
      phyAddr, virtAddr, (u32int)size);

  u32int val = 0;

  if ((phyAddr >= SYS_CTRL_MOD_INTERFACE) && (phyAddr < (SYS_CTRL_MOD_INTERFACE + 36)))
  {
    val = loadInterfaceScm(dev, virtAddr, phyAddr);
  }
  else if ((phyAddr >= SYS_CTRL_MOD_PADCONFS) && (phyAddr < (SYS_CTRL_MOD_PADCONFS + 564)))
  {
    val = loadPadconfsScm(dev, virtAddr, phyAddr);
  }
  else if ((phyAddr >= SYS_CTRL_MOD_GENERAL) && (phyAddr < (SYS_CTRL_MOD_GENERAL + 767)))
  {
    val = loadGeneralScm(dev, virtAddr, phyAddr);
  }
  else if ((phyAddr >= SYS_CTRL_MOD_MEM_WKUP) && (phyAddr < (SYS_CTRL_MOD_MEM_WKUP + 1024)))
  {
    val = loadMemWkupScm(dev, virtAddr, phyAddr);
  }
  else if ((phyAddr >= SYS_CTRL_MOD_PADCONFS_WKUP) && (phyAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + 80)))
  {
    val = loadPadconfsWkupScm(dev, virtAddr, phyAddr);
  }
  else if ((phyAddr >= SYS_CTRL_MOD_GENERAL_WKUP) && (phyAddr < (SYS_CTRL_MOD_GENERAL_WKUP + 31)))
  {
    val = loadGeneralWkupScm(dev, virtAddr, phyAddr);
  }
  else if ((phyAddr >= SYS_CTRL_MOD_PADCONFS_ETK) && (phyAddr <= (SYS_CTRL_MOD_PADCONFS_ETK+SYS_CTRL_MOD_PADCONFS_ETK_SIZE+4)))
  {
    val = loadPadconfsScm(dev, virtAddr, phyAddr);
  }
  else
  {
    printf("%s: load from PA: %#.8x, VA: %#.8x" EOL, __func__, phyAddr, virtAddr);
    DIE_NOW(NULL, "SysControlModule: invalid base module.");
  }

  /*
   * Registers are 8-, 16-, 32-bit accessible with little endianness.
   */
  val = (val >> ((phyAddr & 0x3) * 8));
  switch (size)
  {
    case BYTE:
    {
      val &= 0xFF;
      break;
    }
    case HALFWORD:
    {
      val &= 0xFFFF;
      break;
    }
    default:
      break;
  }
  return val;
}

u32int loadInterfaceScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadInterfaceScm unimplemented.");
  return 0;
}


u32int loadPadconfsScm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - SYS_CTRL_MOD_PADCONFS;

  switch (reg & ~0x3)
  {
    case CONTROL_PADCONF_SDRC_D0:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD0;
      break;
    }
    case CONTROL_PADCONF_SDRC_D2:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD2;
      break;
    }
    case CONTROL_PADCONF_SDRC_D4:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD4;
      break;
    }
    case CONTROL_PADCONF_SDRC_D6:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD6;
      break;
    }
    case CONTROL_PADCONF_SDRC_D8:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD8;
      break;
    }
    case CONTROL_PADCONF_SDRC_D10:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD10;
      break;
    }
    case CONTROL_PADCONF_SDRC_D12:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD12;
      break;
    }
    case CONTROL_PADCONF_SDRC_D14:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD14;
      break;
    }
    case CONTROL_PADCONF_SDRC_D16:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD16;
      break;
    }
    case CONTROL_PADCONF_SDRC_D18:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD18;
      break;
    }
    case CONTROL_PADCONF_SDRC_D20:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD20;
      break;
    }
    case CONTROL_PADCONF_SDRC_D22:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD22;
      break;
    }
    case CONTROL_PADCONF_SDRC_D24:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD24;
      break;
    }
    case CONTROL_PADCONF_SDRC_D26:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD26;
      break;
    }
    case CONTROL_PADCONF_SDRC_D28:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD28;
      break;
    }
    case CONTROL_PADCONF_SDRC_D30:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD30;
      break;
    }
    case CONTROL_PADCONF_SDRC_CLK:
    {
      val = sysCtrlModule->ctrlPadConfSdrcClk;
      break;
    }
    case CONTROL_PADCONF_SDRC_CKE1:
    {
      val = sysCtrlModule->ctrlPadConfSdrcCke1;
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS1:
    {
      val = sysCtrlModule->ctrlPadConfSdrcDqs1;
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS3:
    {
      val = sysCtrlModule->ctrlPadConfSdrcDqs3;
      break;
    }
    case CONTROL_PADCONF_GPMC_A2:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA2;
      break;
    }
    case CONTROL_PADCONF_GPMC_A4:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA4;
      break;
    }
    case CONTROL_PADCONF_GPMC_A6:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA6;
      break;
    }
    case CONTROL_PADCONF_GPMC_A8:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA8;
      break;
    }
    case CONTROL_PADCONF_GPMC_A10:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA10;
      break;
    }
    case CONTROL_PADCONF_GPMC_D1:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD1;
      break;
    }
    case CONTROL_PADCONF_GPMC_D3:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD3;
      break;
    }
    case CONTROL_PADCONF_GPMC_D5:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD5;
      break;
    }
    case CONTROL_PADCONF_GPMC_D7:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD7;
      break;
    }
    case CONTROL_PADCONF_GPMC_D9:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD9;
      break;
    }
    case CONTROL_PADCONF_GPMC_D11:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD11;
      break;
    }
    case CONTROL_PADCONF_GPMC_D13:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD13;
      break;
    }
    case CONTROL_PADCONF_GPMC_D15:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD15;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS1:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs1;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS3:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs3;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS5:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs5;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS7:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs7;
      break;
    }
    case CONTROL_PADCONF_GPMC_NADV_ALE:
    {
      val = sysCtrlModule->ctrlPadConfGpmcAle;
      break;
    }
    case CONTROL_PADCONF_GPMC_NWE:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNwe;
      break;
    }
    case CONTROL_PADCONF_GPMC_NBE1:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNbe1;
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT0:
    {
      val = sysCtrlModule->ctrlPadConfGpmcWait0;
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT2:
    {
      val = sysCtrlModule->ctrlPadConfGpmcWait2;
      break;
    }
    case CONTROL_PADCONF_DSS_PCLK:
    {
      val = sysCtrlModule->ctrlPadConfDssPclk;
      break;
    }
    case CONTROL_PADCONF_DSS_VSYNC:
    {
      val = sysCtrlModule->ctrlPadConfDssVsync;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA0:
    {
      val = sysCtrlModule->ctrlPadConfDssData0;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA2:
    {
      val = sysCtrlModule->ctrlPadConfDssData2;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA4:
    {
      val = sysCtrlModule->ctrlPadConfDssData4;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA6:
    {
      val = sysCtrlModule->ctrlPadConfDssData6;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA8:
    {
      val = sysCtrlModule->ctrlPadConfDssData8;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA10:
    {
      val = sysCtrlModule->ctrlPadConfDssData10;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA12:
    {
      val = sysCtrlModule->ctrlPadConfDssData12;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA14:
    {
      val = sysCtrlModule->ctrlPadConfDssData14;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA16:
    {
      val = sysCtrlModule->ctrlPadConfDssData16;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA18:
    {
      val = sysCtrlModule->ctrlPadConfDssData18;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA20:
    {
      val = sysCtrlModule->ctrlPadConfDssData20;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA22:
    {
      val = sysCtrlModule->ctrlPadConfDssData22;
      break;
    }
    case CONTROL_PADCONF_CAM_HS:
    {
      val = sysCtrlModule->ctrlPadConfCamHs;
      break;
    }
    case CONTROL_PADCONF_CAM_XCLKA:
    {
      val = sysCtrlModule->ctrlPadConfCamXclka;
      break;
    }
    case CONTROL_PADCONF_CAM_FLD:
    {
      val = sysCtrlModule->ctrlPadConfCamFld;
      break;
    }
    case CONTROL_PADCONF_CAM_D1:
    {
      val = sysCtrlModule->ctrlPadConfCamD1;
      break;
    }
    case CONTROL_PADCONF_CAM_D3:
    {
      val = sysCtrlModule->ctrlPadConfCamD3;
      break;
    }
    case CONTROL_PADCONF_CAM_D5:
    {
      val = sysCtrlModule->ctrlPadConfCamD5;
      break;
    }
    case CONTROL_PADCONF_CAM_D7:
    {
      val = sysCtrlModule->ctrlPadConfCamD7;
      break;
    }
    case CONTROL_PADCONF_CAM_D9:
    {
      val = sysCtrlModule->ctrlPadConfCamD9;
      break;
    }
    case CONTROL_PADCONF_CAM_D11:
    {
      val = sysCtrlModule->ctrlPadConfCamD11;
      break;
    }
    case CONTROL_PADCONF_CAM_WEN:
    {
      val = sysCtrlModule->ctrlPadConfCamWen;
      break;
    }
    case CONTROL_PADCONF_CSI2_DX0:
    {
      val = sysCtrlModule->ctrlPadConfCsi2Dx0;
      break;
    }
    case CONTROL_PADCONF_UART3_RTS_SD:
    {
      val = sysCtrlModule->ctrlPadConfUart3RtsSd;
      break;
    }
    case CONTROL_PADCONF_UART3_TX_IRTX:
    {
      val = sysCtrlModule->ctrlPadConfUart3TxIrtx;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_STP:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Stp;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_NXT:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Nxt;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA1:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data1;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA3:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data3;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA5:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data5;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA7:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data7;
      break;
    }
    case CONTROL_PADCONF_I2C1_SDA:
    {
      val = sysCtrlModule->ctrlPadConfI2c1Sda;
      break;
    }
    case CONTROL_PADCONF_I2C2_SDA:
    {
      val = sysCtrlModule->ctrlPadConfI2c2Sda;
      break;
    }
    case CONTROL_PADCONF_I2C3_SDA:
    {
      val = sysCtrlModule->ctrlPadConfI2c3Sda;
      break;
    }
    case CONTROL_PADCONF_CSI2_DX1:
    {
      val = sysCtrlModule->ctrlPadConfCsi2Dx1;
      break;
    }
    case CONTROL_PADCONF_ETK_CLK:
    {
      val = sysCtrlModule->ctrlPadConfEtkClk;
      break;
    }
    case CONTROL_PADCONF_ETK_D0:
    {
      val = sysCtrlModule->ctrlPadConfEtkD0;
      break;
    }
    case CONTROL_PADCONF_ETK_D2:
    {
      val = sysCtrlModule->ctrlPadConfEtkD2;
      break;
    }
    case CONTROL_PADCONF_ETK_D4:
    {
      val = sysCtrlModule->ctrlPadConfEtkD4;
      break;
    }
    case CONTROL_PADCONF_ETK_D6:
    {
      val = sysCtrlModule->ctrlPadConfEtkD6;
      break;
    }
    case CONTROL_PADCONF_ETK_D8:
    {
      val = sysCtrlModule->ctrlPadConfEtkD8;
      break;
    }
    case CONTROL_PADCONF_ETK_D10:
    {
      val = sysCtrlModule->ctrlPadConfEtkD10;
      break;
    }
    case CONTROL_PADCONF_ETK_D12:
    {
      val = sysCtrlModule->ctrlPadConfEtkD12;
      break;
    }
    case CONTROL_PADCONF_ETK_D14:
    {
      val = sysCtrlModule->ctrlPadConfEtkD14;
      break;
    }
    default:
    {
      printf("%s: unimplemented reg addr %#.8x" EOL, __func__, phyAddr);
      DIE_NOW(NULL, "loading non existing/unimplemented register!");
      break;
    }
  }

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, reg, val);
  return val;
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
    case CONTROL_STATUS:
      val = sysCtrlModule->ctrlStatus;
      break;
    case STATUS_REGISTER:
      val = OMAP3530_CHIPID;
      break;
    default:
      printf("loadGeneralScm: unimplemented reg addr %#.8x" EOL, phyAddr);
      DIE_NOW(NULL, "loadGeneralScm loading non existing/unimplemented register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_SCM, "loadGeneralScm reg %x value %#.8x" EOL, reg, val);
  return val;
}


u32int loadMemWkupScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadMemWkupScm unimplemented.");
  return 0;
}


u32int loadPadconfsWkupScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadPadconfsWkupScm unimplemented.");
  return 0;
}


u32int loadGeneralWkupScm(device *dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %#.8x, vAddr %#.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadGeneralWkupScm unimplemented.");
  return 0;
}

/* top store function */
void storeSysCtrlModule(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_SCM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %x, value %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  if ((phyAddr >= SYS_CTRL_MOD_INTERFACE) && (phyAddr < (SYS_CTRL_MOD_INTERFACE + 36)))
  {
    storeInterfaceScm(dev, virtAddr, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_PADCONFS) && (phyAddr < (SYS_CTRL_MOD_PADCONFS + 564)))
  {
    storePadconfsScm(dev, virtAddr, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_GENERAL) && (phyAddr < (SYS_CTRL_MOD_GENERAL + 767)))
  {
    storeGeneralScm(dev, virtAddr, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_MEM_WKUP) && (phyAddr < (SYS_CTRL_MOD_MEM_WKUP + 1024)))
  {
    storeMemWkupScm(dev, virtAddr, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_PADCONFS_WKUP) && (phyAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + 80)))
  {
    storePadconfsWkupScm(dev, virtAddr, phyAddr, value);
  }
  else if ( (phyAddr >= SYS_CTRL_MOD_GENERAL_WKUP) && (phyAddr < (SYS_CTRL_MOD_GENERAL_WKUP + 31)))
  {
    storeGeneralWkupScm(dev, virtAddr, phyAddr, value);
  }
  else
  {
    printf("%s store to pAddr: %#.8x, vAddr %#.8x, aSize %x, value %#.8x" EOL,
        dev->deviceName, phyAddr, virtAddr, (u32int)size, value);
    DIE_NOW(NULL, "Invalid base module.");
  }
}

void storeInterfaceScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeInterfaceScm unimplemented.");
}

void storePadconfsScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storePadconfsScm unimplemented.");
}

void storeGeneralScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeGeneralScm unimplemented.");
}

void storeMemWkupScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeMemWkupScm unimplemented.");
}

void storePadconfsWkupScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storePadconfsWkupScm unimplemented.");
}

void storeGeneralWkupScm(device *dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: Store to address %#.8x, value %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeGeneralWkupScm unimplemented.");
}
