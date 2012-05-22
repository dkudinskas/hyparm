#ifndef __VM__OMAP_35XX__SYS_CONTROL_MODULE_H__
#define __VM__OMAP_35XX__SYS_CONTROL_MODULE_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct SystemControlModule
{
  // registers
  // SYS_CTRL_MOD_INTERFACE      0x48002000 base, 36 bytes length
  u32int ctrlRevision;
  u32int ctrlSysconfig;

  // SYS_CTRL_MOD_PADCONFS       0x48002030 base, 564 bytes length
  u32int ctrlPadConfSdrcD0;
  u32int ctrlPadConfSdrcD2;
  u32int ctrlPadConfSdrcD4;
  u32int ctrlPadConfSdrcD6;
  u32int ctrlPadConfSdrcD8;
  u32int ctrlPadConfSdrcD10;
  u32int ctrlPadConfSdrcD12;
  u32int ctrlPadConfSdrcD14;
  u32int ctrlPadConfSdrcD16;
  u32int ctrlPadConfSdrcD18;
  u32int ctrlPadConfSdrcD20;
  u32int ctrlPadConfSdrcD22;
  u32int ctrlPadConfSdrcD24;
  u32int ctrlPadConfSdrcD26;
  u32int ctrlPadConfSdrcD28;
  u32int ctrlPadConfSdrcD30;
  u32int ctrlPadConfSdrcClk;
  u32int ctrlPadConfSdrcCke1;
  u32int ctrlPadConfSdrcDqs1;
  u32int ctrlPadConfSdrcDqs3;

  u32int ctrlPadConfGpmcA2;
  u32int ctrlPadConfGpmcA4;
  u32int ctrlPadConfGpmcA6;
  u32int ctrlPadConfGpmcA8;
  u32int ctrlPadConfGpmcA10;
  u32int ctrlPadConfGpmcD1;
  u32int ctrlPadConfGpmcD3;
  u32int ctrlPadConfGpmcD5;
  u32int ctrlPadConfGpmcD7;
  u32int ctrlPadConfGpmcD9;
  u32int ctrlPadConfGpmcD11;
  u32int ctrlPadConfGpmcD13;
  u32int ctrlPadConfGpmcD15;
  u32int ctrlPadConfGpmcNcs1;
  u32int ctrlPadConfGpmcNcs3;
  u32int ctrlPadConfGpmcNcs5;
  u32int ctrlPadConfGpmcNcs7;
  u32int ctrlPadConfGpmcAle;
  u32int ctrlPadConfGpmcNwe;
  u32int ctrlPadConfGpmcNbe1;
  u32int ctrlPadConfGpmcWait0;
  u32int ctrlPadConfGpmcWait2;

  u32int ctrlPadConfDssPclk;
  u32int ctrlPadConfDssVsync;
  u32int ctrlPadConfDssData0;
  u32int ctrlPadConfDssData2;
  u32int ctrlPadConfDssData4;
  u32int ctrlPadConfDssData6;
  u32int ctrlPadConfDssData8;
  u32int ctrlPadConfDssData10;
  u32int ctrlPadConfDssData12;
  u32int ctrlPadConfDssData14;
  u32int ctrlPadConfDssData16;
  u32int ctrlPadConfDssData18;
  u32int ctrlPadConfDssData20;
  u32int ctrlPadConfDssData22;

  u32int ctrlPadConfCamHs;
  u32int ctrlPadConfCamXclka;
  u32int ctrlPadConfCamFld;
  u32int ctrlPadConfCamD1;
  u32int ctrlPadConfCamD3;
  u32int ctrlPadConfCamD5;
  u32int ctrlPadConfCamD7;
  u32int ctrlPadConfCamD9;
  u32int ctrlPadConfCamD11;
  u32int ctrlPadConfCamWen;

  u32int ctrlPadConfCsi2Dx0;
  u32int ctrlPadConfCsi2Dx1;

  u32int ctrlPadConfMcbsp2Fsx;
  u32int ctrlPadConfMcbsp2Dr;

  u32int ctrlPadConfMmc1Clk;
  u32int ctrlPadConfMmc1Dat0;
  u32int ctrlPadConfMmc1Dat2;
  u32int ctrlPadConfMmc1Dat4;
  u32int ctrlPadConfMmc1Dat6;

  u32int ctrlPadConfMmc2Clk;
  u32int ctrlPadConfMmc2Dat0;
  u32int ctrlPadConfMmc2Dat2;
  u32int ctrlPadConfMmc2Dat4;
  u32int ctrlPadConfMmc2Dat6;

  u32int ctrlPadConfMcbsp3Dx;
  u32int ctrlPadConfMcbsp3Clkx;

  u32int ctrlPadConfUart2Cts;
  u32int ctrlPadConfUart2Tx;

  u32int ctrlPadConfUart1Tx;
  u32int ctrlPadConfUart1Cts;

  u32int ctrlPadConfMcbsp4Clkx;
  u32int ctrlPadConfMcbsp4Dx;
  u32int ctrlPadConfMcbsp1Clkr;
  u32int ctrlPadConfMcbsp1Dx;
  u32int ctrlPadConfMcbspClks;
  u32int ctrlPadConfMcbsp1Clkx;

  u32int ctrlPadConfUart3RtsSd;
  u32int ctrlPadConfUart3TxIrtx;

  u32int ctrlPadConfHsusb0Stp;
  u32int ctrlPadConfHsusb0Nxt;
  u32int ctrlPadConfHsusb0Data1;
  u32int ctrlPadConfHsusb0Data3;
  u32int ctrlPadConfHsusb0Data5;
  u32int ctrlPadConfHsusb0Data7;

  u32int ctrlPadConfI2c1Sda;
  u32int ctrlPadConfI2c2Sda;
  u32int ctrlPadConfI2c3Sda;

  u32int ctrlPadConfMcspi1Clk;
  u32int ctrlPadConfMcspi1Somi;
  u32int ctrlPadConfMcspi1Cs1;
  u32int ctrlPadConfMcspi1Cs3;

  u32int ctrlPadConfMcspi2Simo;
  u32int ctrlPadConfMcspi2Cs0;

  u32int ctrlPadConfSysNirq;

  u32int ctrlPadConfSad2dMcad0;
  u32int ctrlPadConfSad2dMcad2;
  u32int ctrlPadConfSad2dMcad4;
  u32int ctrlPadConfSad2dMcad6;
  u32int ctrlPadConfSad2dMcad8;
  u32int ctrlPadConfSad2dMcad10;
  u32int ctrlPadConfSad2dMcad12;
  u32int ctrlPadConfSad2dMcad14;
  u32int ctrlPadConfSad2dMcad16;
  u32int ctrlPadConfSad2dMcad18;
  u32int ctrlPadConfSad2dMcad20;
  u32int ctrlPadConfSad2dMcad22;
  u32int ctrlPadConfSad2dMcad24;
  u32int ctrlPadConfSad2dMcad26;
  u32int ctrlPadConfSad2dMcad28;
  u32int ctrlPadConfSad2dMcad30;
  u32int ctrlPadConfSad2dMcad32;
  u32int ctrlPadConfSad2dMcad34;
  u32int ctrlPadConfSad2dMcad36;
  u32int ctrlPadConfSad2dNrespwron;
  u32int ctrlPadConfSad2dArmnirq;
  u32int ctrlPadConfSad2dSpint;
  u32int ctrlPadConfSad2dDmareq0;
  u32int ctrlPadConfSad2dDmareq2;
  u32int ctrlPadConfSad2dNtrst;
  u32int ctrlPadConfSad2dTdo;
  u32int ctrlPadConfSad2dTck;
  u32int ctrlPadConfSad2dMstdby;
  u32int ctrlPadConfSad2dIdleack;
  u32int ctrlPadConfSad2dSwrite;
  u32int ctrlPadConfSad2dSread;
  u32int ctrlPadConfSad2dSbusflag;

  u32int ctrlPadConfEtkClk;
  u32int ctrlPadConfEtkD0;
  u32int ctrlPadConfEtkD2;
  u32int ctrlPadConfEtkD4;
  u32int ctrlPadConfEtkD6;
  u32int ctrlPadConfEtkD8;
  u32int ctrlPadConfEtkD10;
  u32int ctrlPadConfEtkD12;
  u32int ctrlPadConfEtkD14;


  // SYS_CTRL_MOD_GENERAL        0x48002270 base, 767 bytes length
  u32int ctrlPadConfOff;
  u32int ctrlDevConf0;
  u32int ctrlMemDftrw0;
  u32int ctrlMemDftrw1;
  u32int ctrlMsuspendMux0;
  u32int ctrlMsuspendMux1;
  u32int ctrlMsuspendMux2;
  u32int ctrlMsuspendMux3;
  u32int ctrlMsuspendMux4;
  u32int ctrlMsuspendMux5;
  u32int ctrlProtCtrl;
  u32int ctrlDevConf1;
  u32int ctrlCsiRxfe;
  u32int ctrlProtStatus;
  u32int ctrlProtErrStatus;
  u32int ctrlProtErrStatusDebug;
  u32int ctrlStatus;
  u32int ctrlGpStatus;
  u32int ctrlRpubKeyH0;
  u32int ctrlRpubKeyH1;
  u32int ctrlRpubKeyH2;
  u32int ctrlRpubKeyH3;
  u32int ctrlRpubKeyH4;
  // not accessible on the beagle?...  
  /*
  u32int ctrlRandKey0 = 0x;
  u32int ctrlRandKey1 = 0x;
  u32int ctrlRandKey2 = 0x;
  u32int ctrlRandKey3 = 0x;
  u32int ctrlCustKey0 = 0x;
  u32int ctrlCustKey1 = 0x;
  u32int ctrlCustKey2 = 0x;
  u32int ctrlCustKey3 = 0x;
  */
  // .. up to here 
  u32int ctrlUsbConf0;
  u32int ctrlUsbConf1;
  u32int ctrlFuseOpp1Vdd1;
  u32int ctrlFuseOpp2Vdd1;
  u32int ctrlFuseOpp3Vdd1;
  u32int ctrlFuseOpp4Vdd1;
  u32int ctrlFuseOpp5Vdd1;
  u32int ctrlFuseOpp1Vdd2;
  u32int ctrlFuseOpp2Vdd2;
  u32int ctrlFuseOpp3Vdd2;
  u32int ctrlFuseSr;
  u32int ctrlCek0;
  u32int ctrlCek1;
  u32int ctrlCek2;
  u32int ctrlCek3;
  u32int ctrlMsv0;
  u32int ctrlCekBch0;
  u32int ctrlCekBch1;
  u32int ctrlCekBch2;
  u32int ctrlCekBch3;
  u32int ctrlCekBch4;
  u32int ctrlMsvBch0;
  u32int ctrlMsvBch1;
  u32int ctrlSwrv0;
  u32int ctrlSwrv1;
  u32int ctrlSwrv2;
  u32int ctrlSwrv3;
  u32int ctrlSwrv4;
  u32int ctrlIva2Bootaddr;
  u32int ctrlIva2Bootmod;
  u32int ctrlDebobs0;
  u32int ctrlDebobs1;
  u32int ctrlDebobs2;
  u32int ctrlDebobs3;
  u32int ctrlDebobs4;
  u32int ctrlDebobs5;
  u32int ctrlDebobs6;
  u32int ctrlDebobs7;
  u32int ctrlDebobs8;
  u32int ctrlProgIO0;
  u32int ctrlProgIO1;
  u32int ctrlWkupCtrl; // ??? @ off 0x00000A5C
  u32int ctrlDssDpllSpreading;
  u32int ctrlCoreDpllSpreading;
  u32int ctrlPerDpllSpreading;
  u32int ctrlUsbhostDpllSpreading;
  u32int ctrlSdrcSharing;
  u32int ctrlSdrcMcfg0;
  u32int ctrlSdrcMcfg1;
  u32int ctrlModemFwConfLock;
  u32int ctrlModemMemResConf;
  u32int ctrlModemGpmcDtFwReqInfo;
  u32int ctrlModemGpmcDtFwRd;
  u32int ctrlModemGpmcDtFwWr;
  u32int ctrlModemGpmcBootCode;
  u32int ctrlModemSmsRgAtt1;
  u32int ctrlModemSmsRgRdperm1;
  u32int ctrlModemSmsRgWrperm1;
  u32int ctrlModemD2dFwDbgMode;
  u32int ctrlDpfOcmRamFwAddrMatch;
  u32int ctrlDpfOcmRamFwReqinfo;
  u32int ctrlDpfOcmRamFwWr;
  u32int ctrlDpfReg4GpmcFwAddrMatch;
  u32int ctrlDpfReg4GpmcFwReqinfo;
  u32int ctrlDpfReg4GpmcFwWr;
  u32int ctrlDpfReg1Iva2FwAddrMatch;
  u32int ctrlDpfReg1Iva2FwReqinfo;
  u32int ctrlDpfReg1Iva2FwWr;
  u32int ctrlApeFwDefSecLock;
  u32int ctrlOcmRomSecDbg;
  u32int ctrlExtProtCtrl;
  u32int ctrlPbiasLite;
  u32int ctrlCsi;
  u32int ctrlDpfMad2dFwAddrMatch;
  u32int ctrlDpfMad2dFwReqinfo;
  u32int ctrlDpfMad2dFwWr;
  u32int ctrlIdCode; // offs 0x00307F94, phys 0x4830A204 out of range// SYS_CTRL_MOD_MEM_WKUP       0x48002600 base, 1024 bytes length
  // this is just a memory blob of 1k
  // SYS_CTRL_MOD_PADCONFS_WKUP  0x48002A00 base, 80 bytes length
  u32int ctrlPadConfI2c4Scl;
  u32int ctrlPadConfSys32k;
  u32int ctrlPadConfSysNreswarm;
  u32int ctrlPadConfSysBoot1;
  u32int ctrlPadConfSysBoot3;
  u32int ctrlPadConfSysBoot5;
  u32int ctrlPadConfSysOffMode;
  u32int ctrlPadConfJtagNtrst;
  u32int ctrlPadConfJtagTmsTmsc;
  u32int ctrlPadConfJtagEmu0;
  u32int ctrlPadConfSad2dSwakeup;
  u32int ctrlPadConfJtagTdo;

  // SYS_CTRL_MOD_GENERAL_WKUP   0x48002A60 base, 31 bytes length
  // TODO
};


void initSysControlModule(virtualMachine *vm) __cold__;
u32int loadSysCtrlModule(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeSysCtrlModule(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif
