#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END

#include "vm/omap35xx/sysControlModule.h"
#include "vm/omap35xx/sysControlModuleInternals.h"


static u32int loadInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress);
static u32int loadPadconfsScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress);
static u32int loadGeneralScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress);
static u32int loadMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress);
static u32int loadPadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress);
static u32int loadGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress);
static void storeInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value);
static void storePadconfsScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value);
static void storeGeneralScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value);
static void storeMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value);
static void storePadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value);
static void storeGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value);


void initSysControlModule(virtualMachine *vm)
{
  struct SystemControlModule *scm = (struct SystemControlModule *)calloc(1, sizeof(struct SystemControlModule));
  if (scm == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate system control module.");
  }

  DEBUG(VP_OMAP_35XX_SCM, "Initializing system control module at %p" EOL, scm);

  // register default values
  // SYS_CTRL_MOD_INTERFACE      0x48002000 base, 36 bytes length
  scm->ctrlRevision  = 0x10;
  scm->ctrlSysconfig = 0;

  // SYS_CTRL_MOD_PADCONFS       0x48002030 base, 564 bytes length
  scm->ctrlPadConfSdrcD0   = 0x01000100;
  scm->ctrlPadConfSdrcD2   = 0x01000100;
  scm->ctrlPadConfSdrcD4   = 0x01000100;
  scm->ctrlPadConfSdrcD6   = 0x01000100;
  scm->ctrlPadConfSdrcD8   = 0x01000100;
  scm->ctrlPadConfSdrcD10  = 0x01000100;
  scm->ctrlPadConfSdrcD12  = 0x01000100;
  scm->ctrlPadConfSdrcD14  = 0x01000100;
  scm->ctrlPadConfSdrcD16  = 0x01000100;
  scm->ctrlPadConfSdrcD18  = 0x01000100;
  scm->ctrlPadConfSdrcD20  = 0x01000100;
  scm->ctrlPadConfSdrcD22  = 0x01000100;
  scm->ctrlPadConfSdrcD24  = 0x01000100;
  scm->ctrlPadConfSdrcD26  = 0x01000100;
  scm->ctrlPadConfSdrcD28  = 0x01000100;
  scm->ctrlPadConfSdrcD30  = 0x01000100;
  scm->ctrlPadConfSdrcClk  = 0x01000100;
  scm->ctrlPadConfSdrcDqs1 = 0x01000100;
  scm->ctrlPadConfSdrcDqs3 = 0x00000100;

  scm->ctrlPadConfGpmcA2    = 0;
  scm->ctrlPadConfGpmcA4    = 0;
  scm->ctrlPadConfGpmcA6    = 0;
  scm->ctrlPadConfGpmcA8    = 0;
  scm->ctrlPadConfGpmcA10   = 0x01000000;
  scm->ctrlPadConfGpmcD1    = 0x01000100;
  scm->ctrlPadConfGpmcD3    = 0x01000100;
  scm->ctrlPadConfGpmcD5    = 0x01000100;
  scm->ctrlPadConfGpmcD7    = 0x01000100;
  scm->ctrlPadConfGpmcD9    = 0x01000100;
  scm->ctrlPadConfGpmcD11   = 0x01000100;
  scm->ctrlPadConfGpmcD13   = 0x01000100;
  scm->ctrlPadConfGpmcD15   = 0x01000100;
  scm->ctrlPadConfGpmcNcs1  = 0x00180018;
  scm->ctrlPadConfGpmcNcs3  = 0x00180018;
  scm->ctrlPadConfGpmcNcs5  = 0x01010000;
  scm->ctrlPadConfGpmcNcs7  = 0x00000119;
  scm->ctrlPadConfGpmcAle   = 0x01000100;
  scm->ctrlPadConfGpmcNwe   = 0x00000100;
  scm->ctrlPadConfGpmcNbe1  = 0x01000100;
  scm->ctrlPadConfGpmcWait0 = 0x01180118;
  scm->ctrlPadConfGpmcWait2 = 0x01180118;

  scm->ctrlPadConfDssPclk   = 0;
  scm->ctrlPadConfDssVsync  = 0;
  scm->ctrlPadConfDssData0  = 0;
  scm->ctrlPadConfDssData2  = 0;
  scm->ctrlPadConfDssData4  = 0;
  scm->ctrlPadConfDssData6  = 0;
  scm->ctrlPadConfDssData8  = 0;
  scm->ctrlPadConfDssData10 = 0;
  scm->ctrlPadConfDssData12 = 0;
  scm->ctrlPadConfDssData14 = 0;
  scm->ctrlPadConfDssData16 = 0;
  scm->ctrlPadConfDssData18 = 0;
  scm->ctrlPadConfDssData20 = 0;
  scm->ctrlPadConfDssData22 = 0;

  scm->ctrlPadConfCamHs    = 0x01180118;
  scm->ctrlPadConfCamXclka = 0x01180000;
  scm->ctrlPadConfCamFld   = 0x01000004;
  scm->ctrlPadConfCamD1    = 0x01000100;
  scm->ctrlPadConfCamD3    = 0x01000100;
  scm->ctrlPadConfCamD5    = 0x01000100;
  scm->ctrlPadConfCamD7    = 0x01000100;
  scm->ctrlPadConfCamD9    = 0x01000100;
  scm->ctrlPadConfCamD11   = 0x00000100;
  scm->ctrlPadConfCamWen   = 0x00000104;

  scm->ctrlPadConfCsi2Dx0  = 0x01000100;
  scm->ctrlPadConfCsi2Dx1  = 0x01000100;

  scm->ctrlPadConfMcbsp2Fsx = 0x01000100;
  scm->ctrlPadConfMcbsp2Dr  = 0x00000100;

  scm->ctrlPadConfMmc1Clk   = 0x01180018;
  scm->ctrlPadConfMmc1Dat0  = 0x01180018;
  scm->ctrlPadConfMmc1Dat2  = 0x01180018;
  scm->ctrlPadConfMmc1Dat4  = 0x01180018;
  scm->ctrlPadConfMmc1Dat6  = 0x01180018;

  scm->ctrlPadConfMmc2Clk   = 0x011c011c;
  scm->ctrlPadConfMmc2Dat0  = 0x011c011c;
  scm->ctrlPadConfMmc2Dat2  = 0x011c011c;
  scm->ctrlPadConfMmc2Dat4  = 0x011c011c;
  scm->ctrlPadConfMmc2Dat6  = 0x011c011c;

  scm->ctrlPadConfMcbsp3Dx   = 0x01040104;
  scm->ctrlPadConfMcbsp3Clkx = 0x01040104;

  scm->ctrlPadConfUart2Cts = 0x00000118;
  scm->ctrlPadConfUart2Tx  = 0x01040000;

  scm->ctrlPadConfUart1Tx  = 0x00040000;
  scm->ctrlPadConfUart1Cts = 0x01000004;

  scm->ctrlPadConfMcbsp4Clkx = 0x01010101;
  scm->ctrlPadConfMcbsp4Dx   = 0x01010101;
  scm->ctrlPadConfMcbsp1Clkr = 0x001c0004;
  scm->ctrlPadConfMcbsp1Dx   = 0x00040004;
  scm->ctrlPadConfMcbspClks  = 0x00040110;
  scm->ctrlPadConfMcbsp1Clkx = 0x01080004;

  scm->ctrlPadConfUart3RtsSd  = 0x01000000;
  scm->ctrlPadConfUart3TxIrtx = 0x01000000;

  scm->ctrlPadConfHsusb0Stp   = 0x01000018;
  scm->ctrlPadConfHsusb0Nxt   = 0x01000100;
  scm->ctrlPadConfHsusb0Data1 = 0x01000100;
  scm->ctrlPadConfHsusb0Data3 = 0x01000100;
  scm->ctrlPadConfHsusb0Data5 = 0x01000100;
  scm->ctrlPadConfHsusb0Data7 = 0x01180100;

  scm->ctrlPadConfI2c1Sda = 0x011c0118;
  scm->ctrlPadConfI2c2Sda = 0x0118011c;
  scm->ctrlPadConfI2c3Sda = 0x001c0118;

  scm->ctrlPadConfMcspi1Clk  = 0x011c011c;
  scm->ctrlPadConfMcspi1Somi = 0x0108011c;
  scm->ctrlPadConfMcspi1Cs1  = 0x00040008;
  scm->ctrlPadConfMcspi1Cs3  = 0x01130113;

  scm->ctrlPadConfMcspi2Simo = 0x01130113;
  scm->ctrlPadConfMcspi2Cs0  = 0x01130113;

  scm->ctrlPadConfSysNirq    = 0x011c0118;

  scm->ctrlPadConfSad2dMcad0     = 0x01080108;
  scm->ctrlPadConfSad2dMcad2     = 0x01080108;
  scm->ctrlPadConfSad2dMcad4     = 0x01080108;
  scm->ctrlPadConfSad2dMcad6     = 0x01080108;
  scm->ctrlPadConfSad2dMcad8     = 0x01080108;
  scm->ctrlPadConfSad2dMcad10    = 0x01080108;
  scm->ctrlPadConfSad2dMcad12    = 0x01080108;
  scm->ctrlPadConfSad2dMcad14    = 0x01080108;
  scm->ctrlPadConfSad2dMcad16    = 0x01080108;
  scm->ctrlPadConfSad2dMcad18    = 0x01080108;
  scm->ctrlPadConfSad2dMcad20    = 0x01080108;
  scm->ctrlPadConfSad2dMcad22    = 0x01080108;
  scm->ctrlPadConfSad2dMcad24    = 0x01080108;
  scm->ctrlPadConfSad2dMcad26    = 0x01080108;
  scm->ctrlPadConfSad2dMcad28    = 0x01080108;
  scm->ctrlPadConfSad2dMcad30    = 0x01080108;
  scm->ctrlPadConfSad2dMcad32    = 0x01080108;
  scm->ctrlPadConfSad2dMcad34    = 0x01080108;
  scm->ctrlPadConfSad2dMcad36    = 0x01000108;
  scm->ctrlPadConfSad2dNrespwron = 0x01180100;
  scm->ctrlPadConfSad2dArmnirq   = 0x01000100;
  scm->ctrlPadConfSad2dSpint     = 0x01080108;
  scm->ctrlPadConfSad2dDmareq0   = 0x01000100;
  scm->ctrlPadConfSad2dDmareq2   = 0x01000100;
  scm->ctrlPadConfSad2dNtrst     = 0x01000100;
  scm->ctrlPadConfSad2dTdo       = 0x01000100;
  scm->ctrlPadConfSad2dTck       = 0x01000100;
  scm->ctrlPadConfSad2dMstdby    = 0x01000118;
  scm->ctrlPadConfSad2dIdleack   = 0x01000118;
  scm->ctrlPadConfSad2dSwrite    = 0x01000100;
  scm->ctrlPadConfSad2dSread     = 0x01000100;
  scm->ctrlPadConfSad2dSbusflag  = 0x01180100;

  scm->ctrlPadConfSdrcCke1 = 0x01000118;

  scm->ctrlPadConfEtkClk = 0x0013001b;
  scm->ctrlPadConfEtkD0  = 0x01130113;
  scm->ctrlPadConfEtkD2  = 0x01130113;
  scm->ctrlPadConfEtkD4  = 0x01130113;
  scm->ctrlPadConfEtkD6  = 0x01130113;
  scm->ctrlPadConfEtkD8  = 0x01130113;
  scm->ctrlPadConfEtkD10 = 0x00130013;
  scm->ctrlPadConfEtkD12 = 0x01130113;
  scm->ctrlPadConfEtkD14 = 0x01130113;

  // SYS_CTRL_MOD_GENERAL        0x48002270 base, 767 bytes length
  scm->ctrlPadConfOff = 0x00000000;
  scm->ctrlDevConf0 = 0x05000000;
  scm->ctrlMemDftrw0 = 0x00000000;
  scm->ctrlMemDftrw1 = 0x00000000;
  scm->ctrlMsuspendMux0 = 0x00000000;
  scm->ctrlMsuspendMux1 = 0x00000000;
  scm->ctrlMsuspendMux2 = 0x00248000;
  scm->ctrlMsuspendMux3 = 0x00000000;
  scm->ctrlMsuspendMux4 = 0x00000000;
  scm->ctrlMsuspendMux5 = 0x00000000;
  scm->ctrlProtCtrl = 0x00001881;
  scm->ctrlDevConf1 = 0x00000000;
  scm->ctrlCsiRxfe = 0x00000000;
  scm->ctrlProtStatus = 0x00000000;
  scm->ctrlProtErrStatus = 0x00000000;
  scm->ctrlProtErrStatusDebug = 0x00000000;
  scm->ctrlStatus = 0x0000030f;
  scm->ctrlGpStatus = 0x00000000;
  scm->ctrlRpubKeyH0 = 0x00000000;
  scm->ctrlRpubKeyH1 = 0x00000000;
  scm->ctrlRpubKeyH2 = 0x00000000;
  scm->ctrlRpubKeyH3 = 0x00000000;
  scm->ctrlRpubKeyH4 = 0x00000000;
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
  scm->ctrlUsbConf0 = 0x00000000;
  scm->ctrlUsbConf1 = 0x00000000;
  scm->ctrlFuseOpp1Vdd1 = 0x0099bc84;
  scm->ctrlFuseOpp2Vdd1 = 0x009a88c1;
  scm->ctrlFuseOpp3Vdd1 = 0x00aab48a;
  scm->ctrlFuseOpp4Vdd1 = 0x00aba2e6;
  scm->ctrlFuseOpp5Vdd1 = 0x00ab90d3;
  scm->ctrlFuseOpp1Vdd2 = 0x0099be86;
  scm->ctrlFuseOpp2Vdd2 = 0x009a89c4;
  scm->ctrlFuseOpp3Vdd2 = 0x00aac695;
  scm->ctrlFuseSr = 0x00000a0f;
  scm->ctrlCek0 = 0x00000000;
  scm->ctrlCek1 = 0x00000000;
  scm->ctrlCek2 = 0x00000000;
  scm->ctrlCek3 = 0x00000000;
  scm->ctrlMsv0 = 0x00000000;
  scm->ctrlCekBch0 = 0x00000000;
  scm->ctrlCekBch1 = 0x00000000;
  scm->ctrlCekBch2 = 0x00000000;
  scm->ctrlCekBch3 = 0x00000000;
  scm->ctrlCekBch4 = 0x00000000;
  scm->ctrlMsvBch0 = 0x00000000;
  scm->ctrlMsvBch1 = 0x00000000;
  scm->ctrlSwrv0 = 0x02000000;
  scm->ctrlSwrv1 = 0x00000000;
  scm->ctrlSwrv2 = 0x00008000;
  scm->ctrlSwrv3 = 0x00080100;
  scm->ctrlSwrv4 = 0x00200000;
  scm->ctrlIva2Bootaddr = 0x00000000;
  scm->ctrlIva2Bootmod = 0x00000000;
  scm->ctrlDebobs0 = 0x00000000;
  scm->ctrlDebobs1 = 0x00000000;
  scm->ctrlDebobs2 = 0x00000000;
  scm->ctrlDebobs3 = 0x00000000;
  scm->ctrlDebobs4 = 0x00000000;
  scm->ctrlDebobs5 = 0x00000000;
  scm->ctrlDebobs6 = 0x00000000;
  scm->ctrlDebobs7 = 0x00000000;
  scm->ctrlDebobs8 = 0x00000000;
  scm->ctrlProgIO0 = 0x00007fc0;
  scm->ctrlProgIO1 = 0x0002aaaa;
  scm->ctrlWkupCtrl = 0x00000000; // ??? @ off 0x00000A5C
  scm->ctrlDssDpllSpreading = 0x00000040;
  scm->ctrlCoreDpllSpreading = 0x00000040;
  scm->ctrlPerDpllSpreading = 0x00000040;
  scm->ctrlUsbhostDpllSpreading = 0x00000040;
  scm->ctrlSdrcSharing = 0x00002700;
  scm->ctrlSdrcMcfg0 = 0x00300000;
  scm->ctrlSdrcMcfg1 = 0x00300000;
  scm->ctrlModemFwConfLock = 0x00000000;
  scm->ctrlModemMemResConf = 0x00000000;
  scm->ctrlModemGpmcDtFwReqInfo = 0x0000ffff;
  scm->ctrlModemGpmcDtFwRd = 0x0000ffff;
  scm->ctrlModemGpmcDtFwWr = 0x0000ffff;
  scm->ctrlModemGpmcBootCode = 0x00000000;
  scm->ctrlModemSmsRgAtt1 = 0xffffffff;
  scm->ctrlModemSmsRgRdperm1 = 0x0000ffff;
  scm->ctrlModemSmsRgWrperm1 = 0x0000ffff;
  scm->ctrlModemD2dFwDbgMode = 0x00000000;
  scm->ctrlDpfOcmRamFwAddrMatch = 0x00000000;
  scm->ctrlDpfOcmRamFwReqinfo = 0x00000000;
  scm->ctrlDpfOcmRamFwWr = 0x00000000;
  scm->ctrlDpfReg4GpmcFwAddrMatch = 0x00000000;
  scm->ctrlDpfReg4GpmcFwReqinfo = 0x00000000;
  scm->ctrlDpfReg4GpmcFwWr = 0x00000000;
  scm->ctrlDpfReg1Iva2FwAddrMatch = 0x00000000;
  scm->ctrlDpfReg1Iva2FwReqinfo = 0x00000000;
  scm->ctrlDpfReg1Iva2FwWr = 0x00000000;
  scm->ctrlApeFwDefSecLock = 0x00000000;
  scm->ctrlOcmRomSecDbg = 0x00000000;
  scm->ctrlExtProtCtrl = 0x00000002;
  scm->ctrlPbiasLite = 0x00000b87;
  scm->ctrlCsi = 0x03200000;
  scm->ctrlDpfMad2dFwAddrMatch = 0x00000000;
  scm->ctrlDpfMad2dFwReqinfo = 0x00000000;
  scm->ctrlDpfMad2dFwWr = 0x00000000;
  scm->ctrlIdCode = 0x3b7ae02f; // offs 0x00307F94, phys 0x4830A204 out of range

  // SYS_CTRL_MOD_MEM_WKUP       0x48002600 base, 1024 bytes length
  scm->ctrlSaveRestoreMem = (u32int *)calloc(SYS_CTRL_MOD_MEM_WKUP_SIZE, sizeof(u8int));

  // SYS_CTRL_MOD_PADCONFS_WKUP  0x48002A00 base, 80 bytes length
  scm->ctrlPadConfI2c4Scl      = 0x01180118;
  scm->ctrlPadConfSys32k       = 0x01000100;
  scm->ctrlPadConfSysNreswarm  = 0x01040118;
  scm->ctrlPadConfSysBoot1     = 0x01040104;
  scm->ctrlPadConfSysBoot3     = 0x01040104;
  scm->ctrlPadConfSysBoot5     = 0x00040104;
  scm->ctrlPadConfSysOffMode   = 0x01000100;
  scm->ctrlPadConfJtagNtrst    = 0x01000100;
  scm->ctrlPadConfJtagTmsTmsc  = 0x01000100;
  scm->ctrlPadConfJtagEmu0     = 0x01000100;
  scm->ctrlPadConfSad2dSwakeup = 0;
  scm->ctrlPadConfJtagTdo      = 0;

  // SYS_CTRL_MOD_GENERAL_WKUP   0x48002A60 base, 31 bytes length
  // TODO
  vm->sysCtrlModule = scm;
}

/* load function */
u32int loadSysCtrlModule(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  DEBUG(VP_OMAP_35XX_SCM, "%s load from pAddr: %#.8x, vAddr %#.8x, aSize %x" EOL, __func__,
      phyAddr, virtAddr, (u32int)size);

  struct SystemControlModule *scm = context->vm.sysCtrlModule;

  u32int alignedAddr = phyAddr & ~0x3;
  u32int val = 0;

  if ((alignedAddr >= SYS_CTRL_MOD_INTERFACE)
      && (alignedAddr < (SYS_CTRL_MOD_INTERFACE + SYS_CTRL_MOD_INTERFACE_SIZE)))
  {
    val = loadInterfaceScm(scm, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS + SYS_CTRL_MOD_PADCONFS_SIZE)))
  {
    val = loadPadconfsScm(scm, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL + SYS_CTRL_MOD_GENERAL_SIZE)))
  {
    val = loadGeneralScm(scm, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_MEM_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_MEM_WKUP + SYS_CTRL_MOD_MEM_WKUP_SIZE)))
  {
    val = loadMemWkupScm(scm, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + SYS_CTRL_MOD_PADCONFS_WKUP_SIZE)))
  {
    val = loadPadconfsWkupScm(scm, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL_WKUP + SYS_CTRL_MOD_GENERAL_WKUP_SIZE)))
  {
    val = loadGeneralWkupScm(scm, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_ETK)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_ETK + SYS_CTRL_MOD_PADCONFS_ETK_SIZE)))
  {
    val = loadPadconfsScm(scm, phyAddr);
  }
  else
  {
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

static u32int loadInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress)
{
  printf("%s: unimplemented reg addr %#.8x" EOL, __func__, physicalAddress);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}


static u32int loadPadconfsScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress)
{

  u32int value = 0;
  const u32int registerOffset = physicalAddress - SYS_CTRL_MOD_PADCONFS;

  switch (registerOffset & ~0x3)
  {
    case CONTROL_PADCONF_SDRC_D0:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD0;
      break;
    }
    case CONTROL_PADCONF_SDRC_D2:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD2;
      break;
    }
    case CONTROL_PADCONF_SDRC_D4:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD4;
      break;
    }
    case CONTROL_PADCONF_SDRC_D6:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD6;
      break;
    }
    case CONTROL_PADCONF_SDRC_D8:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD8;
      break;
    }
    case CONTROL_PADCONF_SDRC_D10:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD10;
      break;
    }
    case CONTROL_PADCONF_SDRC_D12:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD12;
      break;
    }
    case CONTROL_PADCONF_SDRC_D14:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD14;
      break;
    }
    case CONTROL_PADCONF_SDRC_D16:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD16;
      break;
    }
    case CONTROL_PADCONF_SDRC_D18:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD18;
      break;
    }
    case CONTROL_PADCONF_SDRC_D20:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD20;
      break;
    }
    case CONTROL_PADCONF_SDRC_D22:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD22;
      break;
    }
    case CONTROL_PADCONF_SDRC_D24:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD24;
      break;
    }
    case CONTROL_PADCONF_SDRC_D26:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD26;
      break;
    }
    case CONTROL_PADCONF_SDRC_D28:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD28;
      break;
    }
    case CONTROL_PADCONF_SDRC_D30:
    {
      value = sysCtrlModule->ctrlPadConfSdrcD30;
      break;
    }
    case CONTROL_PADCONF_SDRC_CLK:
    {
      value = sysCtrlModule->ctrlPadConfSdrcClk;
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS1:
    {
      value = sysCtrlModule->ctrlPadConfSdrcDqs1;
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS3:
    {
      value = sysCtrlModule->ctrlPadConfSdrcDqs3;
      break;
    }
    case CONTROL_PADCONF_GPMC_A2:
    {
      value = sysCtrlModule->ctrlPadConfGpmcA2;
      break;
    }
    case CONTROL_PADCONF_GPMC_A4:
    {
      value = sysCtrlModule->ctrlPadConfGpmcA4;
      break;
    }
    case CONTROL_PADCONF_GPMC_A6:
    {
      value = sysCtrlModule->ctrlPadConfGpmcA6;
      break;
    }
    case CONTROL_PADCONF_GPMC_A8:
    {
      value = sysCtrlModule->ctrlPadConfGpmcA8;
      break;
    }
    case CONTROL_PADCONF_GPMC_A10:
    {
      value = sysCtrlModule->ctrlPadConfGpmcA10;
      break;
    }
    case CONTROL_PADCONF_GPMC_D1:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD1;
      break;
    }
    case CONTROL_PADCONF_GPMC_D3:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD3;
      break;
    }
    case CONTROL_PADCONF_GPMC_D5:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD5;
      break;
    }
    case CONTROL_PADCONF_GPMC_D7:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD7;
      break;
    }
    case CONTROL_PADCONF_GPMC_D9:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD9;
      break;
    }
    case CONTROL_PADCONF_GPMC_D11:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD11;
      break;
    }
    case CONTROL_PADCONF_GPMC_D13:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD13;
      break;
    }
    case CONTROL_PADCONF_GPMC_D15:
    {
      value = sysCtrlModule->ctrlPadConfGpmcD15;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS1:
    {
      value = sysCtrlModule->ctrlPadConfGpmcNcs1;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS3:
    {
      value = sysCtrlModule->ctrlPadConfGpmcNcs3;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS5:
    {
      value = sysCtrlModule->ctrlPadConfGpmcNcs5;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS7:
    {
      value = sysCtrlModule->ctrlPadConfGpmcNcs7;
      break;
    }
    case CONTROL_PADCONF_GPMC_NADV_ALE:
    {
      value = sysCtrlModule->ctrlPadConfGpmcAle;
      break;
    }
    case CONTROL_PADCONF_GPMC_NWE:
    {
      value = sysCtrlModule->ctrlPadConfGpmcNwe;
      break;
    }
    case CONTROL_PADCONF_GPMC_NBE1:
    {
      value = sysCtrlModule->ctrlPadConfGpmcNbe1;
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT0:
    {
      value = sysCtrlModule->ctrlPadConfGpmcWait0;
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT2:
    {
      value = sysCtrlModule->ctrlPadConfGpmcWait2;
      break;
    }
    case CONTROL_PADCONF_DSS_PCLK:
    {
      value = sysCtrlModule->ctrlPadConfDssPclk;
      break;
    }
    case CONTROL_PADCONF_DSS_VSYNC:
    {
      value = sysCtrlModule->ctrlPadConfDssVsync;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA0:
    {
      value = sysCtrlModule->ctrlPadConfDssData0;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA2:
    {
      value = sysCtrlModule->ctrlPadConfDssData2;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA4:
    {
      value = sysCtrlModule->ctrlPadConfDssData4;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA6:
    {
      value = sysCtrlModule->ctrlPadConfDssData6;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA8:
    {
      value = sysCtrlModule->ctrlPadConfDssData8;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA10:
    {
      value = sysCtrlModule->ctrlPadConfDssData10;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA12:
    {
      value = sysCtrlModule->ctrlPadConfDssData12;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA14:
    {
      value = sysCtrlModule->ctrlPadConfDssData14;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA16:
    {
      value = sysCtrlModule->ctrlPadConfDssData16;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA18:
    {
      value = sysCtrlModule->ctrlPadConfDssData18;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA20:
    {
      value = sysCtrlModule->ctrlPadConfDssData20;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA22:
    {
      value = sysCtrlModule->ctrlPadConfDssData22;
      break;
    }
    case CONTROL_PADCONF_CAM_HS:
    {
      value = sysCtrlModule->ctrlPadConfCamHs;
      break;
    }
    case CONTROL_PADCONF_CAM_XCLKA:
    {
      value = sysCtrlModule->ctrlPadConfCamXclka;
      break;
    }
    case CONTROL_PADCONF_CAM_FLD:
    {
      value = sysCtrlModule->ctrlPadConfCamFld;
      break;
    }
    case CONTROL_PADCONF_CAM_D1:
    {
      value = sysCtrlModule->ctrlPadConfCamD1;
      break;
    }
    case CONTROL_PADCONF_CAM_D3:
    {
      value = sysCtrlModule->ctrlPadConfCamD3;
      break;
    }
    case CONTROL_PADCONF_CAM_D5:
    {
      value = sysCtrlModule->ctrlPadConfCamD5;
      break;
    }
    case CONTROL_PADCONF_CAM_D7:
    {
      value = sysCtrlModule->ctrlPadConfCamD7;
      break;
    }
    case CONTROL_PADCONF_CAM_D9:
    {
      value = sysCtrlModule->ctrlPadConfCamD9;
      break;
    }
    case CONTROL_PADCONF_CAM_D11:
    {
      value = sysCtrlModule->ctrlPadConfCamD11;
      break;
    }
    case CONTROL_PADCONF_CAM_WEN:
    {
      value = sysCtrlModule->ctrlPadConfCamWen;
      break;
    }
    case CONTROL_PADCONF_CSI2_DX0:
    {
      value = sysCtrlModule->ctrlPadConfCsi2Dx0;
      break;
    }
    case CONTROL_PADCONF_CSI2_DX1:
    {
      value = sysCtrlModule->ctrlPadConfCsi2Dx1;
      break;
    }
    case CONTROL_PADCONF_MCBSP2_FSX:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp2Fsx;
      break;
    }
    case CONTROL_PADCONF_MCBSP2_DR:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp2Dr;
      break;
    }
    case CONTROL_PADCONF_MMC1_CLK:
    {
      value = sysCtrlModule->ctrlPadConfMmc1Clk;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT0:
    {
      value = sysCtrlModule->ctrlPadConfMmc1Dat0;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT2:
    {
      value = sysCtrlModule->ctrlPadConfMmc1Dat2;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT4:
    {
      value = sysCtrlModule->ctrlPadConfMmc1Dat4;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT6:
    {
      value = sysCtrlModule->ctrlPadConfMmc1Dat6;
      break;
    }
    case CONTROL_PADCONF_MMC2_CLK:
    {
      value = sysCtrlModule->ctrlPadConfMmc2Clk;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT0:
    {
      value = sysCtrlModule->ctrlPadConfMmc2Dat0;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT2:
    {
      value = sysCtrlModule->ctrlPadConfMmc2Dat2;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT4:
    {
      value = sysCtrlModule->ctrlPadConfMmc2Dat4;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT6:
    {
      value = sysCtrlModule->ctrlPadConfMmc2Dat6;
      break;
    }
    case CONTROL_PADCONF_MCBSP3_DX:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp3Dx;
      break;
    }
    case CONTROL_PADCONF_MCBSP3_CLKX:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp3Clkx;
      break;
    }
    case CONTROL_PADCONF_UART2_CTS:
    {
      value = sysCtrlModule->ctrlPadConfUart2Cts;
      break;
    }
    case CONTROL_PADCONF_UART2_TX:
    {
      value = sysCtrlModule->ctrlPadConfUart2Tx;
      break;
    }
    case CONTROL_PADCONF_UART1_TX:
    {
      value = sysCtrlModule->ctrlPadConfUart1Tx;
      break;
    }
    case CONTROL_PADCONF_UART1_CTS:
    {
      value = sysCtrlModule->ctrlPadConfUart1Cts;
      break;
    }
    case CONTROL_PADCONF_MCBSP4_CLKX:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp4Clkx;
      break;
    }
    case CONTROL_PADCONF_MCBSP4_DX:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp4Dx;
      break;
    }
    case CONTROL_PADCONF_MCBSP1_CLKR:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp1Clkr;
      break;
    }
    case CONTROL_PADCONF_MCBSP1_DX:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp1Dx;
      break;
    }
    case CONTROL_PADCONF_MCBSP_CLKS:
    {
      value = sysCtrlModule->ctrlPadConfMcbspClks;
      break;
    }
    case CONTROL_PADCONF_MCBSP1_CLKX:
    {
      value = sysCtrlModule->ctrlPadConfMcbsp1Clkx;
      break;
    }
    case CONTROL_PADCONF_UART3_RTS_SD:
    {
      value = sysCtrlModule->ctrlPadConfUart3RtsSd;
      break;
    }
    case CONTROL_PADCONF_UART3_TX_IRTX:
    {
      value = sysCtrlModule->ctrlPadConfUart3TxIrtx;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_STP:
    {
      value = sysCtrlModule->ctrlPadConfHsusb0Stp;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_NXT:
    {
      value = sysCtrlModule->ctrlPadConfHsusb0Nxt;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA1:
    {
      value = sysCtrlModule->ctrlPadConfHsusb0Data1;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA3:
    {
      value = sysCtrlModule->ctrlPadConfHsusb0Data3;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA5:
    {
      value = sysCtrlModule->ctrlPadConfHsusb0Data5;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA7:
    {
      value = sysCtrlModule->ctrlPadConfHsusb0Data7;
      break;
    }
    case CONTROL_PADCONF_I2C1_SDA:
    {
      value = sysCtrlModule->ctrlPadConfI2c1Sda;
      break;
    }
    case CONTROL_PADCONF_I2C2_SDA:
    {
      value = sysCtrlModule->ctrlPadConfI2c2Sda;
      break;
    }
    case CONTROL_PADCONF_I2C3_SDA:
    {
      value = sysCtrlModule->ctrlPadConfI2c3Sda;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CLK:
    {
      value = sysCtrlModule->ctrlPadConfMcspi1Clk;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_SOMI:
    {
      value = sysCtrlModule->ctrlPadConfMcspi1Somi;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CS1:
    {
      value = sysCtrlModule->ctrlPadConfMcspi1Cs1;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CS3:
    {
      value = sysCtrlModule->ctrlPadConfMcspi1Cs3;
      break;
    }
    case CONTROL_PADCONF_MCSPI2_SIMO:
    {
      value = sysCtrlModule->ctrlPadConfMcspi2Simo;
      break;
    }
    case CONTROL_PADCONF_MCSPI2_CS0:
    {
      value = sysCtrlModule->ctrlPadConfMcspi2Cs0;
      break;
    }
    case CONTROL_PADCONF_SYS_NIRQ:
    {
      value = sysCtrlModule->ctrlPadConfSysNirq;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD0:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad0;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD2:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad2;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD4:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad4;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD6:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad6;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD8:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad8;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD10:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad10;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD12:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad12;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD14:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad14;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD16:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad16;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD18:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad18;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD20:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad20;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD22:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad22;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD24:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad24;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD26:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad26;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD28:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad28;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD30:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad30;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD32:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad32;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD34:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad34;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD36:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMcad36;
      break;
    }
    case CONTROL_PADCONF_SAD2D_NRESPWRON:
    {
      value = sysCtrlModule->ctrlPadConfSad2dNrespwron;
      break;
    }
    case CONTROL_PADCONF_SAD2D_ARMNIRQ:
    {
      value = sysCtrlModule->ctrlPadConfSad2dArmnirq;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SPINT:
    {
      value = sysCtrlModule->ctrlPadConfSad2dSpint;
      break;
    }
    case CONTROL_PADCONF_SAD2D_DMAREQ0:
    {
      value = sysCtrlModule->ctrlPadConfSad2dDmareq0;
      break;
    }
    case CONTROL_PADCONF_SAD2D_DMAREQ2:
    {
      value = sysCtrlModule->ctrlPadConfSad2dDmareq2;
      break;
    }
    case CONTROL_PADCONF_SAD2D_NTRST:
    {
      value = sysCtrlModule->ctrlPadConfSad2dNtrst;
      break;
    }
    case CONTROL_PADCONF_SAD2D_TDO:
    {
      value = sysCtrlModule->ctrlPadConfSad2dTdo;
      break;
    }
    case CONTROL_PADCONF_SAD2D_TCK:
    {
      value = sysCtrlModule->ctrlPadConfSad2dTck;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MSTDBY:
    {
      value = sysCtrlModule->ctrlPadConfSad2dMstdby;
      break;
    }
    case CONTROL_PADCONF_SAD2D_IDLEACK:
    {
      value = sysCtrlModule->ctrlPadConfSad2dIdleack;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SWRITE:
    {
      value = sysCtrlModule->ctrlPadConfSad2dSwrite;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SREAD:
    {
      value = sysCtrlModule->ctrlPadConfSad2dSread;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SBUSFLAG:
    {
      value = sysCtrlModule->ctrlPadConfSad2dSbusflag;
      break;
    }
    case CONTROL_PADCONF_SDRC_CKE1:
    {
      value = sysCtrlModule->ctrlPadConfSdrcCke1;
      break;
    }
    case CONTROL_PADCONF_ETK_CLK:
    {
      value = sysCtrlModule->ctrlPadConfEtkClk;
      break;
    }
    case CONTROL_PADCONF_ETK_D0:
    {
      value = sysCtrlModule->ctrlPadConfEtkD0;
      break;
    }
    case CONTROL_PADCONF_ETK_D2:
    {
      value = sysCtrlModule->ctrlPadConfEtkD2;
      break;
    }
    case CONTROL_PADCONF_ETK_D4:
    {
      value = sysCtrlModule->ctrlPadConfEtkD4;
      break;
    }
    case CONTROL_PADCONF_ETK_D6:
    {
      value = sysCtrlModule->ctrlPadConfEtkD6;
      break;
    }
    case CONTROL_PADCONF_ETK_D8:
    {
      value = sysCtrlModule->ctrlPadConfEtkD8;
      break;
    }
    case CONTROL_PADCONF_ETK_D10:
    {
      value = sysCtrlModule->ctrlPadConfEtkD10;
      break;
    }
    case CONTROL_PADCONF_ETK_D12:
    {
      value = sysCtrlModule->ctrlPadConfEtkD12;
      break;
    }
    case CONTROL_PADCONF_ETK_D14:
    {
      value = sysCtrlModule->ctrlPadConfEtkD14;
      break;
    }
    default:
    {
      printf("%s: unimplemented reg addr %#.8x" EOL, __func__, physicalAddress);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, registerOffset, value);
  return value;
}


static u32int loadGeneralScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - SYS_CTRL_MOD_GENERAL;
  u32int value = 0;
  switch (registerOffset)
  {
    case CONTROL_DEVCONF0:
      value = sysCtrlModule->ctrlDevConf0;
      break;
    case CONTROL_DEVCONF1:
      value = sysCtrlModule->ctrlDevConf1;
      break;
    case CONTROL_STATUS:
      value = sysCtrlModule->ctrlStatus;
      break;
    case STATUS_REGISTER:
      value = OMAP3530_CHIPID;
      break;
    case CONTROL_PBIAS_LITE:
      value = sysCtrlModule->ctrlPbiasLite;
      break;
    default:
      printf("loadGeneralScm: unimplemented reg addr %#.8x" EOL, physicalAddress);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  } // switch ends
  DEBUG(VP_OMAP_35XX_SCM, "loadGeneralScm reg %x value %#.8x" EOL, registerOffset, value);
  return value;
}


static u32int loadMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - SYS_CTRL_MOD_MEM_WKUP;
  u32int value = 0;

  value = sysCtrlModule->ctrlSaveRestoreMem[registerOffset];
  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, registerOffset, value);
  return value;
}


static u32int loadPadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - SYS_CTRL_MOD_PADCONFS_WKUP;
  u32int value = 0;
  switch (registerOffset & ~0x3)
  {
    case CONTROL_PADCONF_I2C4_SCL:
    {
      value = sysCtrlModule->ctrlPadConfI2c4Scl;
      break;
    }
    case CONTROL_PADCONF_SYS_32K:
    {
      value = sysCtrlModule->ctrlPadConfSys32k;
      break;
    }
    case CONTROL_PADCONF_SYS_NRESWARM:
    {
      value = sysCtrlModule->ctrlPadConfSysNreswarm;
      break;
    }
    case CONTROL_PADCONF_SYS_BOOT1:
    {
      value = sysCtrlModule->ctrlPadConfSysBoot1;
      break;
    }
    case CONTROL_PADCONF_SYS_BOOT3:
    {
      value = sysCtrlModule->ctrlPadConfSysBoot3;
      break;
    }
    case CONTROL_PADCONF_SYS_BOOT5:
    {
      value = sysCtrlModule->ctrlPadConfSysBoot5;
      break;
    }
    case CONTROL_PADCONF_SYS_OFF_MODE:
    {
      value = sysCtrlModule->ctrlPadConfSysOffMode;
      break;
    }
    case CONTROL_PADCONF_JTAG_NTRST:
    {
      value = sysCtrlModule->ctrlPadConfJtagNtrst;
      break;
    }
    case CONTROL_PADCONF_JTAG_TMS_TMSC:
    {
      value = sysCtrlModule->ctrlPadConfJtagTmsTmsc;
      break;
    }
    case CONTROL_PADCONF_JTAG_EMU0:
    {
      value = sysCtrlModule->ctrlPadConfJtagEmu0;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SWAKEUP:
    {
      value = sysCtrlModule->ctrlPadConfSad2dSwakeup;
      break;
    }
    case CONTROL_PADCONF_JTAG_TDO:
    {
      value = sysCtrlModule->ctrlPadConfJtagTdo;
      break;
    }
    default:
    {
      printf("%s: unimplemented reg addr %#.8x" EOL, __func__, physicalAddress);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, registerOffset, value);
  return value;
}


static u32int loadGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int physicalAddress)
{
  printf("%s: unimplemented reg addr %#.8x" EOL, __func__, physicalAddress);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

/* top store function */
void storeSysCtrlModule(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_SCM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %x, value %#.8x" EOL,
      __func__, phyAddr, virtAddr, (u32int)size, value);

  struct SystemControlModule *scm = context->vm.sysCtrlModule;

  u32int alignedAddr = phyAddr & ~0x3;

  if ((alignedAddr >= SYS_CTRL_MOD_INTERFACE)
      && (alignedAddr < (SYS_CTRL_MOD_INTERFACE + SYS_CTRL_MOD_INTERFACE_SIZE)))
  {
    storeInterfaceScm(scm, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS + SYS_CTRL_MOD_PADCONFS_SIZE)))
  {
    storePadconfsScm(scm, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL + SYS_CTRL_MOD_GENERAL_SIZE)))
  {
    storeGeneralScm(scm, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_MEM_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_MEM_WKUP + SYS_CTRL_MOD_MEM_WKUP_SIZE)))
  {
    storeMemWkupScm(scm, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + SYS_CTRL_MOD_PADCONFS_WKUP_SIZE)))
  {
    storePadconfsWkupScm(scm, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL_WKUP + SYS_CTRL_MOD_GENERAL_WKUP_SIZE)))
  {
    storeGeneralWkupScm(scm, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_ETK)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_ETK + SYS_CTRL_MOD_PADCONFS_ETK_SIZE)))
  {
    storePadconfsScm(scm, phyAddr, value);
  }
  else
  {
    DIE_NOW(NULL, "Invalid base module.");
  }
}

static void storeInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value)
{
  u32int regOffset = phyAddr - SYS_CTRL_MOD_INTERFACE;

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, regOffset, value);
  switch (regOffset)
  {
    case CONTROL_SYSCONFIG:
    {
      if (sysCtrlModule->ctrlSysconfig != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlSysconfig" EOL, __func__);
      }
      break;
    }
    case CONTROL_REVISION:
    {
      DIE_NOW(NULL, "Store to read-only register");
    }
    default:
    {
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storePadconfsScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value)
{
  u32int regOffset = phyAddr - SYS_CTRL_MOD_PADCONFS;

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, regOffset, value);
  switch (regOffset)
  {
    case CONTROL_PADCONF_SDRC_D0:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D2:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D4:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD4" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D6:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD6" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D8:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD8 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD8" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D10:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD10 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD10" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D12:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD12 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD12" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D14:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD14 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD14" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D16:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD16 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD16" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D18:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD18 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD18" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D20:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD20 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD20" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D22:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD22 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD22" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D24:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD24 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD24" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D26:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD26 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD26" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D28:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD28 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD28" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_D30:
    {
      if (sysCtrlModule->ctrlPadConfSdrcD30 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcD30" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_CLK:
    {
      if (sysCtrlModule->ctrlPadConfSdrcClk != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcClk" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS1:
    {
      if (sysCtrlModule->ctrlPadConfSdrcDqs1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcDqs1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS3:
    {
      if (sysCtrlModule->ctrlPadConfSdrcDqs3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcDqs3" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_A2:
    {
      if (sysCtrlModule->ctrlPadConfGpmcA2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcA2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_A4:
    {
      if (sysCtrlModule->ctrlPadConfGpmcA4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcA4" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_A6:
    {
      if (sysCtrlModule->ctrlPadConfGpmcA6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcA6" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_A8:
    {
      if (sysCtrlModule->ctrlPadConfGpmcA8 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcA8" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_A10:
    {
      if (sysCtrlModule->ctrlPadConfGpmcA10 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcA10" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D1:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D3:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD3" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D5:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD5 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD5" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D7:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD7 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD7" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D9:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD9 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD9" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D11:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD11 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD11" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D13:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD13 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD13" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_D15:
    {
      if (sysCtrlModule->ctrlPadConfGpmcD15 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcD15" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS1:
    {
      if (sysCtrlModule->ctrlPadConfGpmcNcs1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcNcs1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS3:
    {
      if (sysCtrlModule->ctrlPadConfGpmcNcs3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcNcs3" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS5:
    {
      if (sysCtrlModule->ctrlPadConfGpmcNcs5 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcNcs5" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS7:
    {
      if (sysCtrlModule->ctrlPadConfGpmcNcs7 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcNcs7" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_NADV_ALE:
    {
      if (sysCtrlModule->ctrlPadConfGpmcAle != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcAle" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_NWE:
    {
      if (sysCtrlModule->ctrlPadConfGpmcNwe != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcNwe" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_NBE1:
    {
      if (sysCtrlModule->ctrlPadConfGpmcNbe1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcNbe1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT0:
    {
      if (sysCtrlModule->ctrlPadConfGpmcWait0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcWait0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT2:
    {
      if (sysCtrlModule->ctrlPadConfGpmcWait2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfGpmcWait2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_PCLK:
    {
      if (sysCtrlModule->ctrlPadConfDssPclk != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssPclk" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_VSYNC:
    {
      if (sysCtrlModule->ctrlPadConfDssVsync != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssVsync" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA0:
    {
      if (sysCtrlModule->ctrlPadConfDssData0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA2:
    {
      if (sysCtrlModule->ctrlPadConfDssData2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA4:
    {
      if (sysCtrlModule->ctrlPadConfDssData4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData4" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA6:
    {
      if (sysCtrlModule->ctrlPadConfDssData6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData6" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA8:
    {
      if (sysCtrlModule->ctrlPadConfDssData8 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData8" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA10:
    {
      if (sysCtrlModule->ctrlPadConfDssData10 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData10" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA12:
    {
      if (sysCtrlModule->ctrlPadConfDssData12 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData12" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA14:
    {
      if (sysCtrlModule->ctrlPadConfDssData14 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData14" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA16:
    {
      if (sysCtrlModule->ctrlPadConfDssData16 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData16" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA18:
    {
      if (sysCtrlModule->ctrlPadConfDssData18 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData18" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA20:
    {
      if (sysCtrlModule->ctrlPadConfDssData20 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData20" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_DSS_DATA22:
    {
      if (sysCtrlModule->ctrlPadConfDssData22 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfDssData22" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_HS:
    {
      if (sysCtrlModule->ctrlPadConfCamHs != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamHs" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_XCLKA:
    {
      if (sysCtrlModule->ctrlPadConfCamXclka != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamXclka" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_FLD:
    {
      if (sysCtrlModule->ctrlPadConfCamFld != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamFld" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_D1:
    {
      if (sysCtrlModule->ctrlPadConfCamD1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamD1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_D3:
    {
      if (sysCtrlModule->ctrlPadConfCamD3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamD3" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_D5:
    {
      if (sysCtrlModule->ctrlPadConfCamD5 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamD5" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_D7:
    {
      if (sysCtrlModule->ctrlPadConfCamD7 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamD7" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_D9:
    {
      if (sysCtrlModule->ctrlPadConfCamD9 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamD9" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_D11:
    {
      if (sysCtrlModule->ctrlPadConfCamD11 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamD11" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CAM_WEN:
    {
      if (sysCtrlModule->ctrlPadConfCamWen != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCamWen" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CSI2_DX0:
    {
      if (sysCtrlModule->ctrlPadConfCsi2Dx0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCsi2Dx0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_CSI2_DX1:
    {
      if (sysCtrlModule->ctrlPadConfCsi2Dx1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfCsi2Dx1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP2_FSX:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp2Fsx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp2Fsx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP2_DR:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp2Dr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp2Dr" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC1_CLK:
    {
      if (sysCtrlModule->ctrlPadConfMmc1Clk != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc1Clk" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT0:
    {
      if (sysCtrlModule->ctrlPadConfMmc1Dat0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc1Dat0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT2:
    {
      if (sysCtrlModule->ctrlPadConfMmc1Dat2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc1Dat2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT4:
    {
      if (sysCtrlModule->ctrlPadConfMmc1Dat4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc1Dat4" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT6:
    {
      if (sysCtrlModule->ctrlPadConfMmc1Dat6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc1Dat6" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC2_CLK:
    {
      if (sysCtrlModule->ctrlPadConfMmc2Clk != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc2Clk" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT0:
    {
      if (sysCtrlModule->ctrlPadConfMmc2Dat0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc2Dat0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT2:
    {
      if (sysCtrlModule->ctrlPadConfMmc2Dat2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc2Dat2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT4:
    {
      if (sysCtrlModule->ctrlPadConfMmc2Dat4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc2Dat4" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT6:
    {
      if (sysCtrlModule->ctrlPadConfMmc2Dat6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMmc2Dat6" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP3_DX:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp3Dx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp3Dx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP3_CLKX:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp3Clkx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp3Clkx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_UART2_CTS:
    {
      if (sysCtrlModule->ctrlPadConfUart2Cts != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfUart2Cts" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_UART2_TX:
    {
      if (sysCtrlModule->ctrlPadConfUart2Tx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfUart2Tx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_UART1_TX:
    {
      if (sysCtrlModule->ctrlPadConfUart1Tx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfUart1Tx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_UART1_CTS:
    {
      if (sysCtrlModule->ctrlPadConfUart1Cts != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfUart1Cts" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP4_CLKX:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp4Clkx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp4Clkx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP4_DX:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp4Dx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp4Dx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP1_CLKR:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp1Clkr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp1Clkr" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP1_DX:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp1Dx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp1Dx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP_CLKS:
    {
      if (sysCtrlModule->ctrlPadConfMcbspClks != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbspClks" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCBSP1_CLKX:
    {
      if (sysCtrlModule->ctrlPadConfMcbsp1Clkx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcbsp1Clkx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_UART3_RTS_SD:
    {
      if (sysCtrlModule->ctrlPadConfUart3RtsSd != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfUart3RtsSd" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_UART3_TX_IRTX:
    {
      if (sysCtrlModule->ctrlPadConfUart3TxIrtx != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfUart3TxIrtx" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_HSUSB0_STP:
    {
      if (sysCtrlModule->ctrlPadConfHsusb0Stp != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfHsusb0Stp" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_HSUSB0_NXT:
    {
      if (sysCtrlModule->ctrlPadConfHsusb0Nxt != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfHsusb0Nxt" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA1:
    {
      if (sysCtrlModule->ctrlPadConfHsusb0Data1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfHsusb0Data1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA3:
    {
      if (sysCtrlModule->ctrlPadConfHsusb0Data3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfHsusb0Data3" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA5:
    {
      if (sysCtrlModule->ctrlPadConfHsusb0Data5 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfHsusb0Data5" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA7:
    {
      if (sysCtrlModule->ctrlPadConfHsusb0Data7 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfHsusb0Data7" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_I2C1_SDA:
    {
      if (sysCtrlModule->ctrlPadConfI2c1Sda != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfI2c1Sda" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_I2C2_SDA:
    {
      if (sysCtrlModule->ctrlPadConfI2c2Sda != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfI2c2Sda" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_I2C3_SDA:
    {
      if (sysCtrlModule->ctrlPadConfI2c3Sda != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfI2c3Sda" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CLK:
    {
      if (sysCtrlModule->ctrlPadConfMcspi1Clk != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcspi1Clk" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCSPI1_SOMI:
    {
      if (sysCtrlModule->ctrlPadConfMcspi1Somi != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcspi1Somi" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CS1:
    {
      if (sysCtrlModule->ctrlPadConfMcspi1Cs1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcspi1Cs1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CS3:
    {
      if (sysCtrlModule->ctrlPadConfMcspi1Cs3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcspi1Cs3" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCSPI2_SIMO:
    {
      if (sysCtrlModule->ctrlPadConfMcspi2Simo != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcspi2Simo" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_MCSPI2_CS0:
    {
      if (sysCtrlModule->ctrlPadConfMcspi2Cs0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfMcspi2Cs0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SYS_NIRQ:
    {
      if (sysCtrlModule->ctrlPadConfSysNirq != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSysNirq" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD0:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD2:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD4:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad4" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD6:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad6" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD8:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad8 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad8" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD10:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad10 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad10" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD12:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad12 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad12" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD14:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad14 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad14" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD16:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad16 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad16" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD18:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad18 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad18" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD20:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad20 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad20" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD22:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad22 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad22" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD24:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad24 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad24" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD26:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad26 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad26" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD28:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad28 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad28" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD30:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad30 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad30" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD32:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad32 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad32" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD34:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad34 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad34" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD36:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMcad36 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMcad36" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_NRESPWRON:
    {
      if (sysCtrlModule->ctrlPadConfSad2dNrespwron != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dNrespwron" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_ARMNIRQ:
    {
      if (sysCtrlModule->ctrlPadConfSad2dArmnirq != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dArmnirq" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_SPINT:
    {
      if (sysCtrlModule->ctrlPadConfSad2dSpint != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dSpint" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_DMAREQ0:
    {
      if (sysCtrlModule->ctrlPadConfSad2dDmareq0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dDmareq0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_DMAREQ2:
    {
      if (sysCtrlModule->ctrlPadConfSad2dDmareq2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dDmareq2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_NTRST:
    {
      if (sysCtrlModule->ctrlPadConfSad2dNtrst != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dNtrst" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_TDO:
    {
      if (sysCtrlModule->ctrlPadConfSad2dTdo != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dTdo" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_TCK:
    {
      if (sysCtrlModule->ctrlPadConfSad2dTck != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dTck" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_MSTDBY:
    {
      if (sysCtrlModule->ctrlPadConfSad2dMstdby != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dMstdby" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_IDLEACK:
    {
      if (sysCtrlModule->ctrlPadConfSad2dIdleack != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dIdleack" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_SWRITE:
    {
      if (sysCtrlModule->ctrlPadConfSad2dSwrite != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dSwrite" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_SREAD:
    {
      if (sysCtrlModule->ctrlPadConfSad2dSread != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dSread" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SAD2D_SBUSFLAG:
    {
      if (sysCtrlModule->ctrlPadConfSad2dSbusflag != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSad2dSbusflag" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_SDRC_CKE1:
    {
      if (sysCtrlModule->ctrlPadConfSdrcCke1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfSdrcCke1" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_CLK:
    {
      if (sysCtrlModule->ctrlPadConfEtkClk != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkClk" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D0:
    {
      if (sysCtrlModule->ctrlPadConfEtkD0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D2:
    {
      if (sysCtrlModule->ctrlPadConfEtkD2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD2" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D4:
    {
      if (sysCtrlModule->ctrlPadConfEtkD4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD4" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D6:
    {
      if (sysCtrlModule->ctrlPadConfEtkD6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD6" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D8:
    {
      if (sysCtrlModule->ctrlPadConfEtkD8 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD8" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D10:
    {
      if (sysCtrlModule->ctrlPadConfEtkD10 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD10" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D12:
    {
      if (sysCtrlModule->ctrlPadConfEtkD12 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD12" EOL, __func__);
      }
      break;
    }
    case CONTROL_PADCONF_ETK_D14:
    {
      if (sysCtrlModule->ctrlPadConfEtkD14 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to sysCtrlModule->ctrlPadConfEtkD14" EOL, __func__);
      }
      break;
    }
    default:
    {
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeGeneralScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value)
{
  u32int regOffset = phyAddr - SYS_CTRL_MOD_GENERAL;

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, regOffset, value);
  switch (regOffset)
  {
    case CONTROL_PADCONF_OFF:
    {
      if (sysCtrlModule->ctrlPadConfOff != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlPadConfOff" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEVCONF0:
    {
      if (sysCtrlModule->ctrlDevConf0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDevConf0" EOL, __func__);
      }
      break;
    }
    case CONTROL_MEM_DFTRW0:
    {
      if (sysCtrlModule->ctrlMemDftrw0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMemDftrw0" EOL, __func__);
      }
      break;
    }
    case CONTROL_MEM_DFTRW1:
    {
      if (sysCtrlModule->ctrlMemDftrw1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMemDftrw1" EOL, __func__);
      }
      break;
    }
    case CONTROL_MSUSPENDMUX_0:
    {
      if (sysCtrlModule->ctrlMsuspendMux0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMsuspendMux0" EOL, __func__);
      }
      break;
    }
    case CONTROL_MSUSPENDMUX_1:
    {
      if (sysCtrlModule->ctrlMsuspendMux1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMsuspendMux1" EOL, __func__);
      }
      break;
    }
    case CONTROL_MSUSPENDMUX_2:
    {
      if (sysCtrlModule->ctrlMsuspendMux2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMsuspendMux2" EOL, __func__);
      }
      break;
    }
    case CONTROL_MSUSPENDMUX_3:
    {
      if (sysCtrlModule->ctrlMsuspendMux3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMsuspendMux3" EOL, __func__);
      }
      break;
    }
    case CONTROL_MSUSPENDMUX_4:
    {
      if (sysCtrlModule->ctrlMsuspendMux4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMsuspendMux4" EOL, __func__);
      }
      break;
    }
    case CONTROL_MSUSPENDMUX_5:
    {
      if (sysCtrlModule->ctrlMsuspendMux5 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlMsuspendMux5" EOL, __func__);
      }
      break;
    }
    case CONTROL_PROT_CTRL:
    {
      if (sysCtrlModule->ctrlProtCtrl != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlProtCtrl" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEVCONF1:
    {
      if (sysCtrlModule->ctrlDevConf1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDevConf1" EOL, __func__);
      }
      break;
    }
    case CONTROL_CSIRXFE:
    {
      if (sysCtrlModule->ctrlCsiRxfe != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlCsiRxfe" EOL, __func__);
      }
      break;
    }
    case CONTROL_PROT_STATUS:
    {
      if (sysCtrlModule->ctrlProtStatus != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlProtStatus" EOL, __func__);
      }
      break;
    }
    case CONTROL_PROT_ERR_STATUS:
    {
      if (sysCtrlModule->ctrlProtErrStatus != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlProtErrStatus" EOL, __func__);
      }
      break;
    }
    case CONTROL_PROT_ERR_STATUS_DEBUG:
    {
      if (sysCtrlModule->ctrlProtErrStatusDebug != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlProtErrStatusDebug" EOL, __func__);
      }
      break;
    }
    case CONTROL_FUSE_OPP1_VDD2:
    {
      if (sysCtrlModule->ctrlFuseOpp1Vdd2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlFuseOpp1Vdd2" EOL, __func__);
      }
      break;
    }
    case CONTROL_FUSE_OPP2_VDD2:
    {
      if (sysCtrlModule->ctrlFuseOpp2Vdd2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlFuseOpp2Vdd2" EOL, __func__);
      }
      break;
    }
    case CONTROL_FUSE_OPP3_VDD2:
    {
      if (sysCtrlModule->ctrlFuseOpp3Vdd2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlFuseOpp3Vdd2" EOL, __func__);
      }
      break;
    }
    case CONTROL_IVA2_BOOTADDR:
    {
      if (sysCtrlModule->ctrlIva2Bootaddr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlIva2Bootaddr" EOL, __func__);
      }
      break;
    }
    case CONTROL_IVA2_BOOTMOD:
    {
      if (sysCtrlModule->ctrlIva2Bootmod != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlIva2Bootmod" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_0:
    {
      if (sysCtrlModule->ctrlDebobs0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs0" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_1:
    {
      if (sysCtrlModule->ctrlDebobs1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs1" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_2:
    {
      if (sysCtrlModule->ctrlDebobs2 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs2" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_3:
    {
      if (sysCtrlModule->ctrlDebobs3 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs3" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_4:
    {
      if (sysCtrlModule->ctrlDebobs4 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs4" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_5:
    {
      if (sysCtrlModule->ctrlDebobs5 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs5" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_6:
    {
      if (sysCtrlModule->ctrlDebobs6 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs6" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_7:
    {
      if (sysCtrlModule->ctrlDebobs7 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs7" EOL, __func__);
      }
      break;
    }
    case CONTROL_DEBOBS_8:
    {
      if (sysCtrlModule->ctrlDebobs8 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDebobs8" EOL, __func__);
      }
      break;
    }
    case CONTROL_PROG_IO0:
    {
      if (sysCtrlModule->ctrlProgIO0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlProgIO0" EOL, __func__);
      }
      break;
    }
    case CONTROL_PROG_IO1:
    {
      if (sysCtrlModule->ctrlProgIO1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlProgIO1" EOL, __func__);
      }
      break;
    }
    case CONTROL_WKUP_CTRL:
    {
      if (sysCtrlModule->ctrlWkupCtrl != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlWkupCtrl" EOL, __func__);
      }
      break;
    }
    case CONTROL_DSS_DPLL_SPREADING:
    {
      if (sysCtrlModule->ctrlDssDpllSpreading != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDssDpllSpreading" EOL, __func__);
      }
      break;
    }
    case CONTROL_CORE_DPLL_SPREADING:
    {
      if (sysCtrlModule->ctrlCoreDpllSpreading != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlCoreDpllSpreading" EOL, __func__);
      }
      break;
    }
    case CONTROL_PER_DPLL_SPREADING:
    {
      if (sysCtrlModule->ctrlPerDpllSpreading != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlPerDpllSpreading" EOL, __func__);
      }
      break;
    }
    case CONTROL_USBHOST_DPLL_SPREADING:
    {
      if (sysCtrlModule->ctrlUsbhostDpllSpreading != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlUsbhostDpllSpreading" EOL, __func__);
      }
      break;
    }
    case CONTROL_SDRC_SHARING:
    {
      if (sysCtrlModule->ctrlSdrcSharing != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlSdrcSharing" EOL, __func__);
      }
      break;
    }
    case CONTROL_SDRC_MCFG0:
    {
      if (sysCtrlModule->ctrlSdrcMcfg0 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlSdrcMcfg0" EOL, __func__);
      }
      break;
    }
    case CONTROL_SDRC_MCFG1:
    {
      if (sysCtrlModule->ctrlSdrcMcfg1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlSdrcMcfg1" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_FW_CONFIGURATION_LOCK:
    {
      if (sysCtrlModule->ctrlModemFwConfLock != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemFwConfLock" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_MEMORY_RESOURCES_CONF:
    {
      if (sysCtrlModule->ctrlModemMemResConf != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemMemResConf" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_GPMC_DT_FW_REQ_INFO:
    {
      if (sysCtrlModule->ctrlModemGpmcDtFwReqInfo != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemGpmcDtFwReqInfo" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_GPMC_DT_FW_RD:
    {
      if (sysCtrlModule->ctrlModemGpmcDtFwRd != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemGpmcDtFwRd" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_GPMC_DT_FW_WR:
    {
      if (sysCtrlModule->ctrlModemGpmcDtFwWr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemGpmcDtFwWr" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_GPMC_BOOT_CODE:
    {
      if (sysCtrlModule->ctrlModemGpmcBootCode != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemGpmcBootCode" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_SMS_RG_ATT1:
    {
      if (sysCtrlModule->ctrlModemSmsRgAtt1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemSmsRgAtt1" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_SMS_RG_RDPERM1:
    {
      if (sysCtrlModule->ctrlModemSmsRgRdperm1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemSmsRgRdperm1" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_SMS_RG_WRPERM1:
    {
      if (sysCtrlModule->ctrlModemSmsRgWrperm1 != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemSmsRgWrperm1" EOL, __func__);
      }
      break;
    }
    case CONTROL_MODEM_D2D_FW_DEBUG_MODE:
    {
      if (sysCtrlModule->ctrlModemD2dFwDbgMode != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlModemD2dFwDbgMode" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_OCM_RAM_FW_ADDR_MATCH:
    {
      if (sysCtrlModule->ctrlDpfOcmRamFwAddrMatch != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfOcmRamFwAddrMatch" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_OCM_RAM_FW_REQINFO:
    {
      if (sysCtrlModule->ctrlDpfOcmRamFwReqinfo != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfOcmRamFwReqinfo" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_OCM_RAM_FW_WR:
    {
      if (sysCtrlModule->ctrlDpfOcmRamFwWr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfOcmRamFwWr" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_REGION4_GPMC_FW_ADDR_MATCH:
    {
      if (sysCtrlModule->ctrlDpfReg4GpmcFwAddrMatch != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfReg4GpmcFwAddrMatch" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_REGION4_GPMC_FW_REQINFO:
    {
      if (sysCtrlModule->ctrlDpfReg4GpmcFwReqinfo != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfReg4GpmcFwReqinfo" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_REGION4_GPMC_FW_WR:
    {
      if (sysCtrlModule->ctrlDpfReg4GpmcFwWr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfReg4GpmcFwWr" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_REGION1_IVA2_FW_ADDR_MATCH:
    {
      if (sysCtrlModule->ctrlDpfReg1Iva2FwAddrMatch != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfReg1Iva2FwAddrMatch" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_REGION1_IVA2_FW_REQINFO:
    {
      if (sysCtrlModule->ctrlDpfReg1Iva2FwReqinfo != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfReg1Iva2FwReqinfo" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_REGION1_IVA2_FW_WR:
    {
      if (sysCtrlModule->ctrlDpfReg1Iva2FwWr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfReg1Iva2FwWr" EOL, __func__);
      }
      break;
    }
    case CONTROL_APE_FW_DEFAULT_SECURE_LOCK:
    {
      if (sysCtrlModule->ctrlApeFwDefSecLock != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlApeFwDefSecLock" EOL, __func__);
      }
      break;
    }
    case CONTROL_OCMROM_SECURE_DEBUG:
    {
      if (sysCtrlModule->ctrlOcmRomSecDbg != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlOcmRomSecDbg" EOL, __func__);
      }
      break;
    }
    case CONTROL_EXT_PROT_CONTROL:
    {
      if (sysCtrlModule->ctrlExtProtCtrl != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlExtProtCtrl" EOL, __func__);
      }
      break;
    }
    case CONTROL_PBIAS_LITE:
    {
      if (sysCtrlModule->ctrlPbiasLite != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlPbiasLite" EOL, __func__);
      }
      break;
    }
    case CONTROL_CSI:
    {
      if (sysCtrlModule->ctrlCsi != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlCsi" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_MAD2D_FW_ADDR_MATCH:
    {
      if (sysCtrlModule->ctrlDpfMad2dFwAddrMatch != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfMad2dFwAddrMatch" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_MAD2D_FW_REQINFO:
    {
      if (sysCtrlModule->ctrlDpfMad2dFwReqinfo != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfMad2dFwReqinfo" EOL, __func__);
      }
      break;
    }
    case CONTROL_DPF_MAD2D_FW_WR:
    {
      if (sysCtrlModule->ctrlDpfMad2dFwWr != value)
      {
        DEBUG(VP_OMAP_35XX_SCM, "%s: ignoring store to ctrlDpfMad2dFwWr" EOL, __func__);
      }
      break;
    }
    case CONTROL_STATUS:
    case CONTROL_GENERAL_PURPOSE_STATUS:
    case CONTROL_RPUB_KEY_H_0:
    case CONTROL_RPUB_KEY_H_1:
    case CONTROL_RPUB_KEY_H_2:
    case CONTROL_RPUB_KEY_H_3:
    case CONTROL_RPUB_KEY_H_4:
    case CONTROL_USB_CONF_0:
    case CONTROL_USB_CONF_1:
    case CONTROL_FUSE_OPP1_VDD1:
    case CONTROL_FUSE_OPP2_VDD1:
    case CONTROL_FUSE_OPP3_VDD1:
    case CONTROL_FUSE_OPP4_VDD1:
    case CONTROL_FUSE_OPP5_VDD1:
    {
      DIE_NOW(NULL, "Store to read-only register");
    }
    default:
    {
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value)
{
  u32int offset = phyAddr - SYS_CTRL_MOD_MEM_WKUP;
  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, offset, value);

  sysCtrlModule->ctrlSaveRestoreMem[offset] = value;
}

static void storePadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, phyAddr, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

static void storeGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, phyAddr, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}
