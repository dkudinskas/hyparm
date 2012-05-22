#ifndef __VM__OMAP_35XX__PRM_H__
#define __VM__OMAP_35XX__PRM_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct PowerAndResetManager
{
  // registers
  // Clock Control registers
  u32int prmClkSelReg;
  u32int prmClkoutCtrlReg;
  // Global reg prm registers
  u32int prmVcSmpsSa;
  u32int prmVcSmpsVolRa;
  u32int prmVcSmpsCmdRa;
  u32int prmVcCmdVal0;
  u32int prmVcCmdVal1;
  u32int prmVcChConf;
  u32int prmVcI2cCfg;
  u32int prmVcBypassVal;
  u32int prmRstCtrl;
  u32int prmRstTime;
  u32int prmRstState;
  u32int prmVoltCtrl;
  u32int prmSramPcharge;
  u32int prmClkSrcCtrl;
  u32int prmObsr;
  u32int prmVoltSetup1;
  u32int prmVoltOffset;
  u32int prmClkSetup;
  u32int prmPolCtrl;
  u32int prmVoltSetup2;
  // IVA2 registers
  u32int prmRstctrlIva2;
  u32int prmRststIva2;
  u32int prmWkdepIva2;
  u32int prmPwstctrlIva2;
  u32int prmPwststIva2;
  // OCP_system_reg registers
  u32int prmRevisionOcp;
  u32int prmSysConfigOcp;
  u32int prmIrqStatusMpuOcp;
  u32int prmIrqEnableMpuOcp;
  // MPU registers
  u32int prmRststMpu;
  u32int prmWkdepMpu;
  u32int prmPwstctrlMpu;
  u32int prmPwststMpu;
  // CORE registers
  u32int prmRststCore;
  u32int prmIva2grpselCore;
  u32int prmPwstctrlCore;
  u32int prmPwststCore;
  u32int prmIva2grpsel3Core;
  // SGX registers
  u32int prmRststSgx;
  u32int prmWkdepSgx;
  u32int prmPwstctrlSgx;
  u32int prmPwststSgx;
  // Wakeup registers
  u32int prmWkenWkup;
  u32int prmMpugrpselWkup;
  u32int prmIva2grpselWkup;
  u32int prmWkstWkup;
  // DSS registers
  u32int prmRststDss;
  u32int prmWkenDss;
  u32int prmWkdepDss;
  u32int prmPwstctrlDss;
  u32int prmPwststDss;
  // CAM registers
  u32int prmRststCam;
  u32int prmWkdepCam;
  u32int prmPwstctrlCam;
  u32int prmPwststCam;
  // PER registers
  u32int prmRststPer;
  u32int prmWkenPer;
  u32int prmMpugrpselPer;
  u32int prmIva2grpselPer;
  u32int prmWkdepPer;
  u32int prmPwstctrlPer;
  u32int prmPwststPer;
  // EMU registers
  u32int prmRststEmu;
  u32int prmPwststEmu;
  // NEON registers
  u32int prmRststNeon;
  u32int prmWkdepNeon;
  u32int prmPwstctrlNeon;
  u32int prmPwststNeon;
  // USBHOST registers
  u32int prmRststUsbhost;
  u32int prmWkdepUsbhost;
  u32int prmPwstctrlUsbhost;
  u32int prmPwststUsbhost;
};


void initPrm(virtualMachine *vm) __cold__;
u32int loadPrm(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storePrm(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__PRM_H__ */
