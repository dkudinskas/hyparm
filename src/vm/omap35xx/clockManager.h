#ifndef __VM_OMAP_35XX__CLOCK_MANAGER_H__
#define __VM_OMAP_35XX__CLOCK_MANAGER_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


struct ClockManager
{
  // registers
  // IVA2_CM registers
  u32int cmFClkEnIva2Reg;
  u32int cmClkEnPllIva2Reg;
  u32int cmIdleStIva2Reg;
  u32int cmIdleStPllIva2Reg;
  u32int cmAutoidlePllIva2Reg;
  u32int cmClkSel1PllIva2Reg;
  u32int cmClkSel2PllIva2Reg;
  u32int cmClkStCtrlIva2Reg;
  u32int cmClkStStIva2Reg;
  // MPU_CM registers
  u32int cmClkEnPllMpuReg;
  u32int cmIdleStMpuReg;
  u32int cmIdleStPllMpuReg;
  u32int cmAutoidlePllMpuReg;
  u32int cmClkSel1PllMpuReg;
  u32int cmClkSel2PllMpuReg;
  u32int cmClkStCtrlMpuReg;
  u32int cmClkStStMpuReg;
  // CORE_CM registers
  u32int cmFclkEn1Core;
  u32int cmFclkEn3Core;
  u32int cmIclkEn1Core;
  u32int cmIclkEn2Core;
  u32int cmIclkEn3Core;
  u32int cmIdleSt1Core;
  u32int cmIdleSt2Core;
  u32int cmIdleSt3Core;
  u32int cmAutoIdle1Core;
  u32int cmAutoIdle2Core;
  u32int cmAutoIdle3Core;
  u32int cmClkSelCore;
  u32int cmClkStCtrl;
  u32int cmClkStSTCore;
  // SGX_CM registers
  u32int cmFclkEnSgx;
  u32int cmIclkEnSgx;
  u32int cmIdleStSgx;
  u32int cmClkSelSgx;
  u32int cmSleepDepSgx;
  u32int cmClkStCtrlSgx;  
  u32int cmClkStSt;
  // WKUP_CM registers
  u32int cmFclkEnWkup;
  u32int cmIclkEnWkup;
  u32int cmIdleStWkup;
  u32int cmAutoIdleWkup;
  u32int cmClkSelWkup;
  // Clock_control_reg_CM registers
  u32int cmClkEnPll;
  u32int cmClkEn2Pll;
  u32int cmIdleStCkGen;
  u32int cmIdleSt2CkGen;
  u32int cmAutoIdlePll;
  u32int cmAutoIdle2Pll;
  u32int cmClkSel1Pll;
  u32int cmClkSel2Pll;
  u32int cmClkSel3Pll;
  u32int cmClkSel4Pll;
  u32int cmClkSel5Pll;
  u32int cmClkoutCtrl;
  // EMU_CM registers
  u32int cmClkSel1Emu;
  u32int cmClkStCtrlEmu;
  u32int cmClkStStEmu;
  u32int cmClkSel2Emu;
  u32int cmClkSel3Emu;
  // DSS_CM registers
  u32int cmFclkEnDss;
  u32int cmIclkEnDss;
  u32int cmIdleStDss;
  u32int cmAutoIdleDss;
  u32int cmClkSelDss; 
  u32int cmSleepDepDss;
  u32int cmClkStCtrlDss;
  u32int cmClkStStDss;
  // CAM_CM registers
  u32int cmFclkEnCam;
  u32int cmIclkEnCam;
  u32int cmIdleStCam;
  u32int cmAutoIdleCam;
  u32int cmClkSelCam;
  u32int cmSleepDepCam;
  u32int cmClkStCtrlCam;
  u32int cmClkStStCam;
  // PER_CM registers
  u32int cmFclkEnPer;
  u32int cmIclkEnPer;
  u32int cmIdleStPer;
  u32int cmAutoIdlePer; 
  u32int cmClkSelPer;
  u32int cmSleepDepPer;
  u32int cmClkStCtrlPer;
  u32int cmClkStStPer;
  // NEON_CM registers
  u32int cmClkStCtrlNeon;
  // USBHOST_CM registers
  u32int cmAutoidleUsb;
  u32int cmClkStCtrlUsb;
};


void initClockManager(virtualMachine *vm) __cold__;
u32int loadClockManager(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeClockManager(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM_OMAP_35XX__CLOCK_MANAGER_H__ */
