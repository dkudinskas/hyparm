#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/clockManager.h"


struct ClockManager * clockMan;


void initClockManager()
{
  // init function: setup device, reset register values to defaults!
  clockMan = (struct ClockManager *)malloc(sizeof(struct ClockManager));
  if (clockMan == 0)
  {
    DIE_NOW(NULL, "Failed to allocate clock manager.");
  }
  memset(clockMan, 0x0, sizeof(struct ClockManager));
  DEBUG(VP_OMAP_35XX_CM, "Initializing Clock manager at %p" EOL, clockMan);
  // IVA2_CM registers
  clockMan->cmFClkEnIva2Reg      = 0x00000001; // IVA2_CLK is enabled
  clockMan->cmClkEnPllIva2Reg    = 0x00000037; // enable DPLL in lock mode, 0.75 MHz--1.0 MHz freq
  clockMan->cmIdleStIva2Reg      = 0x00000001; // IVA2 subsystem is in standby
  clockMan->cmIdleStPllIva2Reg   = 0x00000001; // IVA2 DPLL is locked
  clockMan->cmAutoidlePllIva2Reg = 0x00000000; // auto control is disabled
  clockMan->cmClkSel1PllIva2Reg  = 0x0009680c; // div factor 12, multiplier factor 360, DPLL2_FCLK is CORE_CLK divided by 1
  clockMan->cmClkSel2PllIva2Reg  = 0x00000001; // DPLL2 CLKOUTX2 divided by 1
  clockMan->cmClkStCtrlIva2Reg   = 0x00000000; // auto transition is disabled
  clockMan->cmClkStStIva2Reg     = 0x00000001; // domain clock is active
  // MPU_CM registers
  clockMan->cmClkEnPllMpuReg    = 0x00000037; // enable DPLL in lock mode, 0.75 MHz--1.0 MHz freq
  clockMan->cmIdleStMpuReg      = 0x00000000; // MPU is active
  clockMan->cmIdleStPllMpuReg   = 0x00000001; // DPLL1 is locked
  clockMan->cmAutoidlePllMpuReg = 0x00000000; // auto control is disabled
  clockMan->cmClkSel1PllMpuReg  = 0x0011f40c; // div factor 12, multiplier factor 500, DPLL1_FCLK is CORE_CLK divided by 4
  clockMan->cmClkSel2PllMpuReg  = 0x00000001; // 0x1: DPLL1 CLKOUTX2 divided by 1
  clockMan->cmClkStCtrlMpuReg   = 0x00000000; // auto transition is disabled
  clockMan->cmClkStStMpuReg     = 0x00000001; // domain clock is active
  // CORE_CM registers
  clockMan->cmFclkEn1Core   = 0x03fffe01;
  clockMan->cmFclkEn3Core   = 0x00000000;
  clockMan->cmIclkEn1Core   = 0x3ffffedb;
  clockMan->cmIclkEn2Core   = 0x0000001f;
  clockMan->cmIclkEn3Core   = 0x00000000;
  clockMan->cmIdleSt1Core   = 0xc000001d;
  clockMan->cmIdleSt2Core   = 0x00000000;
  clockMan->cmIdleSt3Core   = 0x0000000d;
  clockMan->cmAutoIdle1Core = 0x00000008;
  clockMan->cmAutoIdle2Core = 0x00000000;
  clockMan->cmAutoIdle3Core = 0x00000000;
  clockMan->cmClkSelCore    = 0x0000030a;
  clockMan->cmClkStCtrl     = 0x00000000;
  clockMan->cmClkStSTCore   = 0x00000007;
  // SGX_CM registers
  clockMan->cmFclkEnSgx    = 0x00000000;
  clockMan->cmIclkEnSgx    = 0x00000000;
  clockMan->cmIdleStSgx    = 0x00000001;
  clockMan->cmClkSelSgx    = 0x00000002;
  clockMan->cmSleepDepSgx  = 0x00000000;
  clockMan->cmClkStCtrlSgx = 0x00000000;
  clockMan->cmClkStSt      = 0x00000000;
  // WKUP_CM registers
  clockMan->cmFclkEnWkup   = 0x00000029;
  clockMan->cmIclkEnWkup   = 0x0000003f;
  clockMan->cmIdleStWkup   = 0x00000200;
  clockMan->cmAutoIdleWkup = 0x00000000;
  clockMan->cmClkSelWkup   = 0x00000015;
  // Clock_control_reg_CM registers
  clockMan->cmClkEnPll    = 0x00370037; // DPLL3, DPLL4 in lock mode @ 0.75 MHz--1.0 MHz
  clockMan->cmClkEn2Pll   = 0x00000011; // DPLL5 in low power stop mode @ 0.75 MHz--1.0 MHz
  clockMan->cmIdleStCkGen = 0x00001e3f; // DPLL3 and DPLL4 locked, 48M, 96M, 54M, 12M, DSS1_ALWON,
                                // DPLL4_M3X2_CLK and DPLL4_M2X2_CLK FCLK's and CAM_MCLK active
  clockMan->cmIdleSt2CkGen = 0x00000000;
  clockMan->cmAutoIdlePll  = 0x00000000;
  clockMan->cmAutoIdle2Pll = 0x00000000;
  clockMan->cmClkSel1Pll   = 0x094c0c00;
  clockMan->cmClkSel2Pll   = 0x0001b00c;
  clockMan->cmClkSel3Pll   = 0x00000009;
  clockMan->cmClkSel4Pll   = 0x00000000;
  clockMan->cmClkSel5Pll   = 0x00000001;
  clockMan->cmClkoutCtrl   = 0x00000003;
  // EMU_CM registers
  clockMan->cmClkSel1Emu   = 0x03020a50;
  clockMan->cmClkStCtrlEmu = 0x00000002;
  clockMan->cmClkStStEmu   = 0x00000001;
  clockMan->cmClkSel2Emu   = 0x00000000;
  clockMan->cmClkSel3Emu   = 0x00000000;
  // DSS_CM registers
  clockMan->cmFclkEnDss    = 0x00000005;
  clockMan->cmIclkEnDss    = 0x00000001;
  clockMan->cmIdleStDss    = 0x00000001;
  clockMan->cmAutoIdleDss  = 0x00000000;
  clockMan->cmClkSelDss    = 0x00001002;
  clockMan->cmSleepDepDss  = 0x00000000;
  clockMan->cmClkStCtrlDss = 0x00000000;
  clockMan->cmClkStStDss   = 0x00000001;
  // CAM_CM registers
  clockMan->cmFclkEnCam    = 0x00000001;
  clockMan->cmIclkEnCam    = 0x00000001;
  clockMan->cmIdleStCam    = 0x00000001;
  clockMan->cmAutoIdleCam  = 0x00000000;
  clockMan->cmClkSelCam    = 0x00000004;
  clockMan->cmSleepDepCam  = 0x00000000;
  clockMan->cmClkStCtrlCam = 0x00000000;
  clockMan->cmClkStStCam   = 0x00000001;
  // PER_CM registers
  clockMan->cmFclkEnPer    = 0x0003ffff;
  clockMan->cmIclkEnPer    = 0x0003ffff;
  clockMan->cmIdleStPer    = 0x00000000;
  clockMan->cmAutoIdlePer  = 0x00000000;
  clockMan->cmClkSelPer    = 0x000000ff;
  clockMan->cmSleepDepPer  = 0x00000000;
  clockMan->cmClkStCtrlPer = 0x00000000;
  clockMan->cmClkStStPer   = 0x00000001;
}

/*************************************************************************
 *                           Load  Functions                             *
 *************************************************************************/
u32int loadClockManager(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  //We care about the real pAddr of the entry, not its vAddr
  DEBUG(VP_OMAP_35XX_CM, "%s load from pAddr: %.8x, vAddr %.8x, aSize %x" EOL, dev->deviceName,
      phyAddr, virtAddr, (u32int)size);

  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "CM: invalid access size.");
  }
  u32int val = 0;
  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case IVA2_CM:
      val = loadIva2Cm(dev, virtAddr, phyAddr);
      break;
    case OCP_System_Reg_CM:
      val = loadOcpSystemCm(dev, virtAddr, phyAddr);
      break;
    case MPU_CM:
      val = loadMpuCm(dev, virtAddr, phyAddr);
      break;
    case CORE_CM:
      val = loadCoreCm(dev, virtAddr, phyAddr);
      break;
    case SGX_CM:
      val = loadSgxCm(dev, virtAddr, phyAddr);
      break;
    case WKUP_CM:
      val = loadWkupCm(dev, virtAddr, phyAddr);
      break;
    case Clock_Control_Reg_CM:
      val = loadClockControlCm(dev, virtAddr, phyAddr);
      break;
    case DSS_CM:
      val = loadDssCm(dev, virtAddr, phyAddr);
      break;
    case CAM_CM:
      val = loadCamCm(dev, virtAddr, phyAddr);
      break;
    case PER_CM:
      val = loadPerCm(dev, virtAddr, phyAddr);
      break;
    case EMU_CM:
      val = loadEmuCm(dev, virtAddr, phyAddr);
      break;
    case Global_Reg_CM:
      val = loadGlobalRegCm(dev, virtAddr, phyAddr);
      break;
    case NEON_CM:
      val = loadNeonCm(dev, virtAddr, phyAddr);
      break;
    case USBHOST_CM:
      val = loadUsbHostCm(dev, virtAddr, phyAddr);
      break;
    default:
      DIE_NOW(NULL, "CM: invalid base module.");
  } // switch ends
  return val;
} // loadClockManager


u32int loadIva2Cm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - IVA2_CM;
  switch (reg)
  {
    case CM_FCLKEN_IVA2:
      val = clockMan->cmFClkEnIva2Reg;
      break;
    case CM_CLKEN_PLL_IVA2:
      val = clockMan->cmClkEnPllIva2Reg;
      break;
    case CM_IDLEST_IVA2:
      val = clockMan->cmIdleStIva2Reg;
      break;
    case CM_IDLEST_PLL_IVA2:
      val = clockMan->cmIdleStPllIva2Reg;
      break;
    case CM_AUTOIDLE_PLL_IVA2:
      val = clockMan->cmAutoidlePllIva2Reg;
      break;
    case CM_CLKSEL1_PLL_IVA2:
      val = clockMan->cmClkSel1PllIva2Reg;
      break;
    case CM_CLKSEL2_PLL_IVA2:
      val = clockMan->cmClkSel2PllIva2Reg;
      break;
    case CM_CLKSTCTRL_IVA2:
      val = clockMan->cmClkStCtrlIva2Reg;
      break;
    case CM_CLKSTST_IVA2:
      val = clockMan->cmClkStStIva2Reg;
      break;
    default:
      DIE_NOW(NULL, "loadIva2Cm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadIva2Cm reg %#x value %#.8x" EOL, reg, val);
  return val;
}

u32int loadOcpSystemCm(device * dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %.8x, vAddr %.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadOcpSystemCm unimplemented.");
  return 0;
}

u32int loadMpuCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - MPU_CM;
  switch (reg)
  {
    case CM_CLKEN_PLL_MPU:
      val = clockMan->cmClkEnPllMpuReg;
      break;
    case CM_IDLEST_MPU:
      val = clockMan->cmIdleStMpuReg;
      break;
    case CM_IDLEST_PLL_MPU:
      val = clockMan->cmIdleStPllMpuReg;
      break;
    case CM_AUTOIDLE_PLL_MPU:
      val = clockMan->cmAutoidlePllMpuReg;
      break;
    case CM_CLKSEL1_PLL_MPU:
      val = clockMan->cmClkSel1PllMpuReg;
      break;
    case CM_CLKSEL2_PLL_MPU:
      val = clockMan->cmClkSel2PllMpuReg;
      break;
    case CM_CLKSTCTRL_MPU:
      val = clockMan->cmClkStCtrlMpuReg;
      break;
    case CM_CLKSTST_MPU:
      val = clockMan->cmClkStStMpuReg;
      break;
    default:
      DIE_NOW(NULL, "loadMpuCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadMpuCm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadCoreCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - CORE_CM;
  switch (reg)
  {
    case CM_FCLKEN1_CORE:
      val = clockMan->cmFclkEn1Core;
      break;
    case CM_FCLKEN3_CORE:
      val = clockMan->cmFclkEn3Core;
      break;
    case CM_ICLKEN1_CORE:
      val = clockMan->cmIclkEn1Core;
      break;
    case CM_ICLKEN2_CORE:
      val = clockMan->cmIclkEn2Core;
      break;
    case CM_ICLKEN3_CORE:
      val = clockMan->cmIclkEn3Core;
      break;
    case CM_IDLEST1_CORE:
      val = clockMan->cmIdleSt1Core;
      break;
    case CM_IDLEST2_CORE:
      val = clockMan->cmIdleSt2Core;
      break;
    case CM_IDLEST3_CORE:
      val = clockMan->cmIdleSt3Core;
      break;
    case CM_AUTOIDLE1_CORE:
      val = clockMan->cmAutoIdle1Core;
      break;
    case CM_AUTOIDLE2_CORE:
      val = clockMan->cmAutoIdle2Core;
      break;
    case CM_AUTOIDLE3_CORE:
      val = clockMan->cmAutoIdle3Core;
      break;
    case CM_CLKSEL_CORE:
      val = clockMan->cmClkSelCore;
      break;
    case CM_CLKSTCTRL_CORE:
      val = clockMan->cmClkStCtrl;
      break;
    case CM_CLKSTST_CORE:
      val = clockMan->cmClkStSTCore;
      break;
    default:
      DIE_NOW(NULL, "loadCoreCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadCoreCm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadSgxCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - SGX_CM;
  switch (reg)
  {
    case CM_FCLKEN_SGX:
      val = clockMan->cmFclkEnSgx;
      break;
    case CM_ICLKEN_SGX:
      val = clockMan->cmIclkEnSgx;
      break;
    case CM_IDLEST_SGX:
      val = clockMan->cmIdleStSgx;
      break;
    case CM_CLKSEL_SGX:
      val = clockMan->cmClkSelSgx;
      break;
    case CM_SLEEPDEP_SGX:
      val = clockMan->cmSleepDepSgx;
      break;
    case CM_CLKSTCTRL_SGX:
      val = clockMan->cmClkStCtrlSgx;
      break;
    case CM_CLKSTST_SGX:
      val = clockMan->cmClkStSt;
      break;
    default:
      DIE_NOW(NULL, "loadSgxCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadSgxCm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadWkupCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - WKUP_CM;
  switch (reg)
  {
    case CM_FCLKEN_WKUP:
      val = clockMan->cmFclkEnWkup;
      break;
    case CM_ICLKEN_WKUP:
      val = clockMan->cmIclkEnWkup;
      break;
    case CM_IDLEST_WKUP:
      val = clockMan->cmIdleStWkup;
      break;
    case CM_AUTOIDLE_WKUP:
      val = clockMan->cmAutoIdleWkup;
      break;
    case CM_CLKSEL_WKUP:
      val = clockMan->cmClkSelWkup;
      break;
    case CM_CLKSTCTRL_WKUP:
      DEBUG(VP_OMAP_35XX_CM, "CLKMAN: warn: wkupCm load from undocumented reg (offs 0x48)" EOL);
      val = 0;
      break;
    default:
      printf("loadWkupCm reg %x" EOL, reg);
      DIE_NOW(NULL, "loadWkupCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadWkupCm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadClockControlCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - Clock_Control_Reg_CM;
  switch (reg)
  {
    case CM_CLKEN_PLL:
      val = clockMan->cmClkEnPll;
      break;
    case CM_CLKEN2_PLL:
      val = clockMan->cmClkEn2Pll;
      break;
    case CM_UNDOCUMENTED:
      val = 0;
      DEBUG(VP_OMAP_35XX_CM, "loadClockControlCm warning: loading from undocumented reg %#x "
          "returning zero" EOL, reg);
      break;
    case CM_IDLEST_CKGEN:
      val = clockMan->cmIdleStCkGen;
      break;
    case CM_IDLEST2_CKGEN:
      val = clockMan->cmIdleSt2CkGen;
      break;
    case CM_AUTOIDLE_PLL:
      val = clockMan->cmAutoIdlePll;
      break;
    case CM_AUTOIDLE2_PLL:
      val = clockMan->cmAutoIdle2Pll;
      break;
    case CM_CLKSEL1_PLL:
      val = clockMan->cmClkSel1Pll;
      break;
    case CM_CLKSEL2_PLL:
      val = clockMan->cmClkSel2Pll;
      break;
    case CM_CLKSEL3_PLL:
      val = clockMan->cmClkSel3Pll;
      break;
    case CM_CLKSEL4_PLL:
      val = clockMan->cmClkSel4Pll;
      break;
    case CM_CLKSEL5_PLL:
      val = clockMan->cmClkSel5Pll;
      break;
    case CM_CLKOUT_CTRL:
      val = clockMan->cmClkoutCtrl;
      break;
    default:
      printf("loadClockControlCm reg %x" EOL, reg);
      DIE_NOW(NULL, "loadClockControlCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadClockControlCm reg %#x value %#.8x" EOL, reg, val);
  return val;
}

u32int loadDssCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - DSS_CM;
  switch (reg)
  {
    case CM_FCLKEN_DSS:
      val = clockMan->cmFclkEnDss;
      break;
    case CM_ICLKEN_DSS:
      val = clockMan->cmIclkEnDss;
      break;
    case CM_IDLEST_DSS:
      val = clockMan->cmIdleStDss;
      break;
    case CM_AUTOIDLE_DSS:
      val = clockMan->cmAutoIdleDss;
      break;
    case CM_CLKSEL_DSS:
      val = clockMan->cmClkSelDss;
      break;
    case CM_SLEEPDEP_DSS:
      val = clockMan->cmSleepDepDss;
      break;
    case CM_CLKSTCTRL_DSS:
      val = clockMan->cmClkStCtrlDss;
      break;
    case CM_CLKSTST_DSS:
      val = clockMan->cmClkStStDss;
      break;
    default:
      DIE_NOW(NULL, "loadDssCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadDssCm reg %#x value %#.8x" EOL, reg, val);
  return val;
}

u32int loadCamCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - CAM_CM;
  switch (reg)
  {
    case CM_FCLKEN_DSS:
      val = clockMan->cmFclkEnCam;
      break;
    case CM_ICLKEN_DSS:
      val = clockMan->cmIclkEnCam;
      break;
    case CM_IDLEST_DSS:
      val = clockMan->cmIdleStCam;
      break;
    case CM_AUTOIDLE_DSS:
      val = clockMan->cmAutoIdleCam;
      break;
    case CM_CLKSEL_DSS:
      val = clockMan->cmClkSelCam;
      break;
    case CM_SLEEPDEP_DSS:
      val = clockMan->cmSleepDepCam;
      break;
    case CM_CLKSTCTRL_DSS:
      val = clockMan->cmClkStCtrlCam;
      break;
    case CM_CLKSTST_DSS:
      val = clockMan->cmClkStStCam;
      break;
    default:
      DIE_NOW(NULL, "loadCamCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadCamCm reg %#x value %#.8x" EOL, reg, val);
  return val;
}

u32int loadPerCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - PER_CM;
  switch (reg)
  {
    case CM_FCLKEN_PER:
      val = clockMan->cmFclkEnPer;
      break;
    case CM_ICLKEN_PER:
      val = clockMan->cmIclkEnPer;
      break;
    case CM_IDLEST_PER:
      val = clockMan->cmIdleStPer;
      break;
    case CM_AUTOIDLE_PER:
      val = clockMan->cmAutoIdlePer;
      break;
    case CM_CLKSEL_PER:
      val = clockMan->cmClkSelPer;
      break;
    case CM_SLEEPDEP_PER:
      val = clockMan->cmSleepDepPer;
      break;
    case CM_CLKSTCTRL_PER:
      val = clockMan->cmClkStCtrlPer;
      break;
    case CM_CLKSTST_PER:
      val = clockMan->cmClkStStPer;
      break;
    default:
      DIE_NOW(NULL, "loadPerCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadPerCm reg %#x value %#.8x" EOL, reg, val);
  return val;
}

u32int loadEmuCm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - EMU_CM;
  switch (reg)
  {
    case CM_CLKSEL1_EMU:
      val = clockMan->cmClkSel1Emu;
      break;
    case CM_CLKSTCTRL_EMU:
      val = clockMan->cmClkStCtrlEmu;
      break;
    case CM_CLKSTST_EMU:
      val = clockMan->cmClkStStEmu;
      break;
    case CM_CLKSEL2_EMU:
      val = clockMan->cmClkSel2Emu;
      break;
    case CM_CLKSEL3_EMU:
      val = clockMan->cmClkSel3Emu;
      break;
    default:
      DIE_NOW(NULL, "loadEmuCm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_CM, "loadEmuCm reg %#x value %#.8x" EOL, reg, val);
  return val;
}

u32int loadGlobalRegCm(device * dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %.8x, vAddr %.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadGlobalRegCm unimplemented.");
  return 0;
}

u32int loadNeonCm(device * dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %.8x, vAddr %.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadNeonCm unimplemented.");
  return 0;
}

u32int loadUsbHostCm(device * dev, u32int address, u32int phyAddr)
{
  printf("%s load from pAddr: %.8x, vAddr %.8x" EOL, dev->deviceName, phyAddr, address);
  DIE_NOW(NULL, "loadUsbHostCm unimplemented.");
  return 0;
}

/*************************************************************************
 *                           Store Functions                             *
 *************************************************************************/
void storeClockManager(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  DEBUG(VP_OMAP_35XX_CM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);
  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case CORE_CM:
      storeCoreCm(dev, virtAddr, phyAddr, value);
      break;
    case WKUP_CM:
      storeWkupCm(dev, virtAddr, phyAddr, value);
      break;
    case PER_CM:
      storePerCm(dev, virtAddr, phyAddr, value);
      break;
    case IVA2_CM:
      storeIva2Cm(dev, virtAddr, phyAddr, value);
      break;
    case OCP_System_Reg_CM:
      storeOcpSystemCm(dev, virtAddr, phyAddr, value);
      break;
    case MPU_CM:
      storeMpuCm(dev, virtAddr, phyAddr, value);
      break;
    case SGX_CM:
      storeSgxCm(dev, virtAddr, phyAddr, value);
      break;
    case Clock_Control_Reg_CM:
      storeClockControlCm(dev, virtAddr, phyAddr, value);
      break;
    case DSS_CM:
      storeDssCm(dev, virtAddr, phyAddr, value);
      break;
    case CAM_CM:
      storeCamCm(dev, virtAddr, phyAddr, value);
      break;
    case EMU_CM:
      storeEmuCm(dev, virtAddr, phyAddr, value);
      break;
    case Global_Reg_CM:
      storeGlobalRegCm(dev, virtAddr, phyAddr, value);
      break;
    case NEON_CM:
      storeNeonCm(dev, virtAddr, phyAddr, value);
      break;
    case USBHOST_CM:
      storeUsbHostCm(dev, virtAddr, phyAddr, value);
      break;
    default:
      DIE_NOW(NULL, "CM: store to invalid base module.");
  } // switch ends
} // storeClockManager

void storeIva2Cm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeIva2Cm unimplemented.");
  return;
}

void storeOcpSystemCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeOcpSystemCm unimplemented.");
  return;
}

void storeMpuCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeMpuCm unimplemented.");
  return;
}

void storeCoreCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_CM, "%s: store to address %#.8x val %#.8x" EOL, dev->deviceName, address,
      value);
  u32int reg = phyAddr - CORE_CM;
  switch (reg)
  {
    case CM_ICLKEN1_CORE:
      if (clockMan->cmIclkEn1Core != value)
      {
        DIE_NOW(NULL, "storeCoreCm CM_ICLKEN1_CORE unimplemented.");
      }
      break;
    case CM_FCLKEN1_CORE:
      if (clockMan->cmFclkEn1Core != value)
      {
        DIE_NOW(NULL, "storeCoreCm CM_FCLKEN1_CORE unimplemented.");
      }
      break;
    case CM_FCLKEN3_CORE:
    case CM_ICLKEN2_CORE:
    case CM_ICLKEN3_CORE:
    case CM_IDLEST1_CORE:
    case CM_IDLEST2_CORE:
    case CM_IDLEST3_CORE:
    case CM_AUTOIDLE1_CORE:
    case CM_AUTOIDLE2_CORE:
    case CM_AUTOIDLE3_CORE:
    case CM_CLKSEL_CORE:
    case CM_CLKSTCTRL_CORE:
    case CM_CLKSTST_CORE:
      DIE_NOW(NULL, "storeCoreCm unimplemented.");
      break;
    default:
      DIE_NOW(NULL, "storeCoreCm storing non existing register!");
  } // switch ends
  return;
}

void storeSgxCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %#.8x val %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeSgxCm unimplemented.");
  return;
}

void storeWkupCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_CM, "%s: store to address %#.8x val %#.8x" EOL, dev->deviceName, address,
      value);
  u32int reg = phyAddr - WKUP_CM;
  switch (reg)
  {
    case CM_FCLKEN_WKUP:
      // clear reserved bits... and check meaningful bit values
      value = value & ~CM_FCLKEN_WKUP_RESERVED;
      if (clockMan->cmFclkEnWkup != value)
      {
        if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_WDT2) == CM_FCLKEN_WKUP_WDT2) &&
             ((value & CM_FCLKEN_WKUP_WDT2) == 0) )
        {
          printf("CM: warn: cmWkup disabling watchdog timer 2 functional clock." EOL);
        }
        else if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_WDT2) == 0) &&
             ((value & CM_FCLKEN_WKUP_WDT2) == CM_FCLKEN_WKUP_WDT2) )
        {
          printf("CM: warn: cmWkup enabling watchdog timer 2 functional clock." EOL);
        }
        if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_GPIO1) == CM_FCLKEN_WKUP_GPIO1) &&
             ((value & CM_FCLKEN_WKUP_GPIO1) == 0) )
        {
          printf("CM: warn: cmWkup disabling gpio1 functional clock." EOL);
        }
        else if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_GPIO1) == 0) &&
             ((value & CM_FCLKEN_WKUP_GPIO1) == CM_FCLKEN_WKUP_GPIO1) )
        {
          printf("CM: warn: cmWkup enabling gpio1 functional clock." EOL);
        }
        if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_ENGPT1) == CM_FCLKEN_WKUP_ENGPT1) &&
             ((value & CM_FCLKEN_WKUP_ENGPT1) == 0) )
        {
          printf("CM: warn: cmWkup disabling gptimer1 functional clock." EOL);
        }
        else if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_ENGPT1) == 0) &&
             ((value & CM_FCLKEN_WKUP_ENGPT1) == CM_FCLKEN_WKUP_ENGPT1) )
        {
          printf("CM: warn: cmWkup enabling gptimer1 functional clock." EOL);
        }
        clockMan->cmFclkEnWkup = value;
      }
      break;
    case CM_ICLKEN_WKUP:
      // clear reserved bits... and check meaningful bit values
      value = value & ~CM_ICLKEN_WKUP_RESERVED;
      if (clockMan->cmIclkEnWkup != value)
      {
        if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_WDT2) == CM_ICLKEN_WKUP_WDT2) &&
             ((value & CM_ICLKEN_WKUP_WDT2) == 0) )
        {
          printf("CM: warn: cmWkup disabling watchdog timer 2 interface clock." EOL);
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_WDT2) == 0) &&
             ((value & CM_ICLKEN_WKUP_WDT2) == CM_ICLKEN_WKUP_WDT2) )
        {
          printf("CM: warn: cmWkup enabling watchdog timer 2 interface clock." EOL);
        }
        if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_GPIO1) == CM_ICLKEN_WKUP_GPIO1) &&
             ((value & CM_ICLKEN_WKUP_GPIO1) == 0) )
        {
          printf("CM: warn: cmWkup disabling gpio1 interface clock." EOL);
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_GPIO1) == 0) &&
             ((value & CM_ICLKEN_WKUP_GPIO1) == CM_ICLKEN_WKUP_GPIO1) )
        {
          printf("CM: warn: cmWkup enabling gpio1 interface clock." EOL);
        }
        if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_ENGPT1) == CM_ICLKEN_WKUP_ENGPT1) &&
             ((value & CM_ICLKEN_WKUP_ENGPT1) == 0) )
        {
          printf("CM: warn: cmWkup disabling gptimer1 interface clock." EOL);
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_ENGPT1) == 0) &&
             ((value & CM_ICLKEN_WKUP_ENGPT1) == CM_ICLKEN_WKUP_ENGPT1) )
        {
          printf("CM: warn: cmWkup enabling gptimer1 interface clock." EOL);
        }
        clockMan->cmFclkEnWkup = value;
      }
      break;
    case CM_IDLEST_WKUP:
      if (clockMan->cmIdleStWkup != value)
      {
        DIE_NOW(NULL, " storeWkupCm unimplemented store to reg cmIdleStWkup");
      }
      break;
    case CM_AUTOIDLE_WKUP:
      if (clockMan->cmAutoIdleWkup != value)
      {
        DIE_NOW(NULL, " storeWkupCm unimplemented store to reg cmAutoIdleWkup");
      }
      break;
    case CM_CLKSEL_WKUP:
#define CM_CLKSEL_WKUP_GPT1            0x00000001
      // clear reserved bits... and check meaningful bit values
      value = value & ~CM_CLKSEL_WKUP_RESERVED1;
      if (clockMan->cmClkSelWkup != value)
      {
        if ( (clockMan->cmClkSelWkup & CM_CLKSEL_WKUP_RM) != (value & CM_CLKSEL_WKUP_RM) )
        {
          printf("CM: warn: cmWkup reset module clock set to %x" EOL, (value & CM_CLKSEL_WKUP_RM) >> 1);
        }
        if ( ((clockMan->cmClkSelWkup & CM_CLKSEL_WKUP_GPT1) == 0) &&
             ((value & CM_CLKSEL_WKUP_GPT1) == CM_CLKSEL_WKUP_GPT1) )
        {
          printf("CM: warn: cmWkup set gptimer1 clock to system clock" EOL);
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_CLKSEL_WKUP_GPT1) == CM_CLKSEL_WKUP_GPT1) &&
             ((value & CM_CLKSEL_WKUP_GPT1) == 0) )
        {
          printf("CM: warn: cmWkup set gptimer1 clock to 32kHz clock" EOL);
        }
      }
      clockMan->cmClkSelWkup = value;
      break;
    case CM_CLKSTCTRL_WKUP:
      DEBUG(VP_OMAP_35XX_CM, "%s WARN: store to undocumented register CM_CLKSTCTRL_WKUP" EOL,
          dev->deviceName);
      break;
    default:
      DIE_NOW(NULL, "storeWkupCm storing non existing register!");
  } // switch ends
  return;
}

void storeClockControlCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_CM, "%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  u32int reg = phyAddr - Clock_Control_Reg_CM;
  switch (reg)
  {
    case CM_CLKEN_PLL:
      printf("Store to CM_CLKEN_PLL val %.8x" EOL, value);
      clockMan->cmClkEnPll = value;
      break;
    case CM_AUTOIDLE_PLL:
      printf("Store to CM_AUTOIDLE_PLL val %.8x" EOL, value);
      clockMan->cmAutoIdlePll = value;
      break;
    case CM_CLKEN2_PLL:
    case CM_IDLEST_CKGEN:
    case CM_IDLEST2_CKGEN:
    case CM_AUTOIDLE2_PLL:
    case CM_CLKSEL1_PLL:
    case CM_CLKSEL2_PLL:
    case CM_CLKSEL3_PLL:
    case CM_CLKSEL4_PLL:
    case CM_CLKSEL5_PLL:
    case CM_CLKOUT_CTRL:
      printf("%s: store to VA %.8x, PA %.8x, value %.8x" EOL, dev->deviceName, address, phyAddr, value);
      DIE_NOW(NULL, "storeClockControlCm store to unimplemented register register!");
      break;
    default:
      printf("%s: store to VA %.8x, PA %.8x, value %.8x" EOL, dev->deviceName, address, phyAddr, value);
      DIE_NOW(NULL, "storeClockControlCm storing non existing register!");
  } // switch ends
}

void storeDssCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeDssCm unimplemented.");
  return;
}

void storeCamCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %#.8x val %#.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeCamCm unimplemented.");
  return;
}

void storePerCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_CM, "%s: store to address %#.8x val %#.8x" EOL, dev->deviceName, address,
      value);
  u32int reg = phyAddr - PER_CM;
  switch (reg)
  {
    case CM_FCLKEN_PER:
      if (clockMan->cmFclkEnPer != value)
      {
        DIE_NOW(NULL, "storePerCm unimplemented store to reg cmFclkEnPer");
      }
      break;
    case CM_ICLKEN_PER:
      if (clockMan->cmIclkEnPer != value)
      {
        DIE_NOW(NULL, "storePerCm unimplemented store to reg cmIclkEnPer");
      }
      break;
    case CM_IDLEST_PER:
      if (clockMan->cmIdleStPer != value)
      {
        DIE_NOW(NULL, "storePerCm unimplemented store to reg cmIdleStPer");
      }
      break;
    case CM_AUTOIDLE_PER:
      if (clockMan->cmAutoIdlePer != value)
      {
        DIE_NOW(NULL, "storePerCm unimplemented store to reg cmAutoIdlePer");
      }
      break;
    case CM_CLKSEL_PER:
      if (clockMan->cmClkSelPer != value)
      {
        DIE_NOW(NULL, "storePerCm unimplemented store to reg cmClkSelPer");
      }
      break;
    case CM_SLEEPDEP_PER:
      if (clockMan->cmSleepDepPer != value)
      {
        DIE_NOW(NULL, "storePerCm unimplemented store to reg cmSleepDepPer");
      }
      break;
    case CM_CLKSTCTRL_PER:
      if (value & 0x2)
      {
        printf("CMper: WARNING software forced wakeup, deliver interrupt?" EOL);
      }
      clockMan->cmClkStCtrlPer = value;
      break;
    case CM_CLKSTST_PER:
      if (clockMan->cmClkStStPer != value)
      {
        DIE_NOW(NULL, "storePerCm unimplemented store to reg cmClkStStPer");
      }
      break;
    default:
      DIE_NOW(NULL, "storePerCm storing non existing register!");
  } // switch ends
}

void storeEmuCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeEmuCm unimplemented.");
  return;
}

void storeGlobalRegCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeGlobalRegCm unimplemented.");
  return;
}

void storeNeonCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeNeonCm unimplemented.");
  return;
}

void storeUsbHostCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("%s: store to address %.8x val %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, "storeUsbHostCm unimplemented.");
  return;
}
