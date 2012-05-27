#include "common/bit.h"
#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/omap35xx.h"
#include "vm/omap35xx/clockManager.h"
#include "vm/omap35xx/clockManagerInternals.h"


static u32int loadCamCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadClockControlCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadCoreCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadDssCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadEmuCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadIva2Cm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadMpuCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadNeonCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadPerCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadSgxCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadUsbHostCm(struct ClockManager *cm, u32int physicalAddress);
static u32int loadWkupCm(struct ClockManager *cm, u32int physicalAddress);

static void storeCamCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeClockControlCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeCoreCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeDssCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeEmuCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeIva2Cm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeMpuCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeNeonCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storePerCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeSgxCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeUsbHostCm(struct ClockManager *cm, u32int physicalAddress, u32int value);
static void storeWkupCm(struct ClockManager *cm, u32int physicalAddress, u32int value);


void initClockManager(virtualMachine *vm)
{
  struct ClockManager *const cm = (struct ClockManager *)calloc(1, sizeof(struct ClockManager));
  if (cm == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate clock manager.");
  }
  vm->clockMan = cm;

  DEBUG(VP_OMAP_35XX_CM, "Initializing Clock manager at %p" EOL, cm);

  // IVA2_CM registers
  cm->cmFClkEnIva2Reg      = 0x00000001; // IVA2_CLK is enabled
  cm->cmClkEnPllIva2Reg    = 0x00000037; // enable DPLL in lock mode, 0.75 MHz--1.0 MHz freq
  cm->cmIdleStIva2Reg      = 0x00000001; // IVA2 subsystem is in standby
  cm->cmIdleStPllIva2Reg   = 0x00000001; // IVA2 DPLL is locked
  cm->cmAutoidlePllIva2Reg = 0x00000000; // auto control is disabled
  cm->cmClkSel1PllIva2Reg  = 0x0009680c; // div factor 12, multiplier factor 360, DPLL2_FCLK is CORE_CLK divided by 1
  cm->cmClkSel2PllIva2Reg  = 0x00000001; // DPLL2 CLKOUTX2 divided by 1
  cm->cmClkStCtrlIva2Reg   = 0x00000000; // auto transition is disabled
  cm->cmClkStStIva2Reg     = 0x00000001; // domain clock is active
  // MPU_CM registers
  cm->cmClkEnPllMpuReg    = 0x00000037; // enable DPLL in lock mode, 0.75 MHz--1.0 MHz freq
  cm->cmIdleStMpuReg      = 0x00000000; // MPU is active
  cm->cmIdleStPllMpuReg   = 0x00000001; // DPLL1 is locked
  cm->cmAutoidlePllMpuReg = 0x00000000; // auto control is disabled
  cm->cmClkSel1PllMpuReg  = 0x0011f40c; // div factor 12, multiplier factor 500, DPLL1_FCLK is CORE_CLK divided by 4
  cm->cmClkSel2PllMpuReg  = 0x00000001; // 0x1: DPLL1 CLKOUTX2 divided by 1
  cm->cmClkStCtrlMpuReg   = 0x00000000; // auto transition is disabled
  cm->cmClkStStMpuReg     = 0x00000001; // domain clock is active
  // CORE_CM registers
  cm->cmFclkEn1Core   = 0x03fffe01;
  cm->cmFclkEn3Core   = 0x00000000;
  cm->cmIclkEn1Core   = 0x3ffffedb;
  cm->cmIclkEn2Core   = 0x0000001f;
  cm->cmIclkEn3Core   = 0x00000000;
#ifdef CONFIG_GUEST_ANDROID
  cm->cmIdleSt1Core   = 0x8000001d;
#else
  cm->cmIdleSt1Core   = 0xc000001d;
#endif
  cm->cmIdleSt2Core   = 0x00000000;
  cm->cmIdleSt3Core   = 0x0000000d;
  cm->cmAutoIdle1Core = 0x00000008;
  cm->cmAutoIdle2Core = 0x00000000;
  cm->cmAutoIdle3Core = 0x00000000;
  cm->cmClkSelCore    = 0x0000030a;
  cm->cmClkStCtrl     = 0x00000000;
  cm->cmClkStSTCore   = 0x00000007;
  // SGX_CM registers
  cm->cmFclkEnSgx    = 0x00000000;
  cm->cmIclkEnSgx    = 0x00000000;
  cm->cmIdleStSgx    = 0x00000001;
  cm->cmClkSelSgx    = 0x00000002;
  cm->cmSleepDepSgx  = 0x00000000;
  cm->cmClkStCtrlSgx = 0x00000000;
  cm->cmClkStSt      = 0x00000000;
  // WKUP_CM registers
  cm->cmFclkEnWkup   = 0x00000029;
  cm->cmIclkEnWkup   = 0x0000003f;
  cm->cmIdleStWkup   = 0x00000200;
  cm->cmAutoIdleWkup = 0x00000000;
  cm->cmClkSelWkup   = 0x00000015;
  // Clock_control_reg_CM registers
  cm->cmClkEnPll    = 0x00370037; // DPLL3, DPLL4 in lock mode @ 0.75 MHz--1.0 MHz
  cm->cmClkEn2Pll   = 0x00000011; // DPLL5 in low power stop mode @ 0.75 MHz--1.0 MHz
  cm->cmIdleStCkGen = 0x00001e3f; // DPLL3 and DPLL4 locked, 48M, 96M, 54M, 12M, DSS1_ALWON,
                                // DPLL4_M3X2_CLK and DPLL4_M2X2_CLK FCLK's and CAM_MCLK active
  cm->cmIdleSt2CkGen = 0x00000000;
  cm->cmAutoIdlePll  = 0x00000000;
  cm->cmAutoIdle2Pll = 0x00000000;
  cm->cmClkSel1Pll   = 0x094c0c00;
  cm->cmClkSel2Pll   = 0x0001b00c;
  cm->cmClkSel3Pll   = 0x00000009;
  cm->cmClkSel4Pll   = 0x00000000;
  cm->cmClkSel5Pll   = 0x00000001;
  cm->cmClkoutCtrl   = 0x00000003;
  // EMU_CM registers
  cm->cmClkSel1Emu   = 0x03020a50;
  cm->cmClkStCtrlEmu = 0x00000002;
  cm->cmClkStStEmu   = 0x00000001;
  cm->cmClkSel2Emu   = 0x00000000;
  cm->cmClkSel3Emu   = 0x00000000;
  // DSS_CM registers
  cm->cmFclkEnDss    = 0x00000005;
  cm->cmIclkEnDss    = 0x00000001;
  cm->cmIdleStDss    = 0x00000001;
  cm->cmAutoIdleDss  = 0x00000000;
  cm->cmClkSelDss    = 0x00001002;
  cm->cmSleepDepDss  = 0x00000000;
  cm->cmClkStCtrlDss = 0x00000000;
  cm->cmClkStStDss   = 0x00000001;
  // CAM_CM registers
  cm->cmFclkEnCam    = 0x00000001;
  cm->cmIclkEnCam    = 0x00000001;
  cm->cmIdleStCam    = 0x00000001;
  cm->cmAutoIdleCam  = 0x00000000;
  cm->cmClkSelCam    = 0x00000004;
  cm->cmSleepDepCam  = 0x00000000;
  cm->cmClkStCtrlCam = 0x00000000;
  cm->cmClkStStCam   = 0x00000001;
  // PER_CM registers
  cm->cmFclkEnPer    = 0x0003ffff;
  cm->cmIclkEnPer    = 0x0003ffff;
  cm->cmIdleStPer    = 0x00000000;
  cm->cmAutoIdlePer  = 0x00000000;
  cm->cmClkSelPer    = 0x000000ff;
  cm->cmSleepDepPer  = 0x00000000;
  cm->cmClkStCtrlPer = 0x00000000;
  cm->cmClkStStPer   = 0x00000001;
  // NEON_CM registers
  cm->cmClkStCtrlNeon = 0;
  // USBHOST_CM registers
  cm->cmAutoidleUsb  = 0;
  cm->cmClkStCtrlUsb = 0;
}

u32int loadClockManager(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int physicalAddress)
{
  //We care about the real pAddr of the entry, not its vAddr
  DEBUG(VP_OMAP_35XX_CM, "%s load from pAddr: %#.8x, vAddr %#.8x, aSize %x" EOL, dev->deviceName,
      physicalAddress, virtAddr, (u32int)size);

  ASSERT(size == WORD, ERROR_BAD_ACCESS_SIZE);

  struct ClockManager *cm = context->vm.clockMan;
  u32int base = physicalAddress & 0xFFFFFF00;
  switch (base)
  {
    case IVA2_CM:
      return loadIva2Cm(cm, physicalAddress);
    case MPU_CM:
      return loadMpuCm(cm, physicalAddress);
    case CORE_CM:
      return loadCoreCm(cm, physicalAddress);
    case SGX_CM:
      return loadSgxCm(cm, physicalAddress);
    case WKUP_CM:
      return loadWkupCm(cm, physicalAddress);
    case Clock_Control_Reg_CM:
      return loadClockControlCm(cm, physicalAddress);
    case DSS_CM:
      return loadDssCm(cm, physicalAddress);
    case CAM_CM:
      return loadCamCm(cm, physicalAddress);
    case PER_CM:
      return loadPerCm(cm, physicalAddress);
    case EMU_CM:
      return loadEmuCm(cm, physicalAddress);
    case NEON_CM:
      return loadNeonCm(cm, physicalAddress);
    case USBHOST_CM:
      return loadUsbHostCm(cm, physicalAddress);
    case Global_Reg_CM:
    case OCP_System_Reg_CM:
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    default:
      DIE_NOW(NULL, "CM: invalid base module.");
  }
}

static u32int loadCamCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - CAM_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_FCLKEN_CAM:
    {
      value = cm->cmFclkEnCam;
      break;
    }
    case CM_ICLKEN_CAM:
    {
      value = cm->cmIclkEnCam;
      break;
    }
    case CM_IDLEST_CAM:
    {
      value = cm->cmIdleStCam;
      break;
    }
    case CM_AUTOIDLE_CAM:
    {
      value = cm->cmAutoIdleCam;
      break;
    }
    case CM_CLKSEL_CAM:
    {
      value = cm->cmClkSelCam;
      break;
    }
    case CM_SLEEPDEP_CAM:
    {
      value = cm->cmSleepDepCam;
      break;
    }
    case CM_CLKSTCTRL_CAM:
    {
      value = cm->cmClkStCtrlCam;
      break;
    }
    case CM_CLKSTST_CAM:
    {
      value = cm->cmClkStStCam;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadCamCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadClockControlCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - Clock_Control_Reg_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_CLKEN_PLL:
    {
      value = cm->cmClkEnPll;
      break;
    }
    case CM_CLKEN2_PLL:
    {
      value = cm->cmClkEn2Pll;
      break;
    }
    case CM_UNDOCUMENTED:
    {
      DEBUG(VP_OMAP_35XX_CM, "loadClockControlCm: undocumented register %x" EOL, registerOffset);
      break;
    }
    case CM_IDLEST_CKGEN:
    {
      value = cm->cmIdleStCkGen;
      break;
    }
    case CM_IDLEST2_CKGEN:
    {
      value = cm->cmIdleSt2CkGen;
      break;
    }
    case CM_AUTOIDLE_PLL:
    {
      value = cm->cmAutoIdlePll;
      break;
    }
    case CM_AUTOIDLE2_PLL:
    {
      value = cm->cmAutoIdle2Pll;
      break;
    }
    case CM_CLKSEL1_PLL:
    {
      value = cm->cmClkSel1Pll;
      break;
    }
    case CM_CLKSEL2_PLL:
    {
      value = cm->cmClkSel2Pll;
      break;
    }
    case CM_CLKSEL3_PLL:
    {
      value = cm->cmClkSel3Pll;
      break;
    }
    case CM_CLKSEL4_PLL:
    {
      value = cm->cmClkSel4Pll;
      break;
    }
    case CM_CLKSEL5_PLL:
    {
      value = cm->cmClkSel5Pll;
      break;
    }
    case CM_CLKOUT_CTRL:
    {
      value = cm->cmClkoutCtrl;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadClockControlCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadCoreCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - CORE_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_FCLKEN1_CORE:
    {
      value = cm->cmFclkEn1Core;
      break;
    }
    case CM_FCLKEN3_CORE:
    {
      value = cm->cmFclkEn3Core;
      break;
    }
    case CM_ICLKEN1_CORE:
    {
      value = cm->cmIclkEn1Core;
      break;
    }
    case CM_ICLKEN2_CORE:
    {
      value = cm->cmIclkEn2Core;
      break;
    }
    case CM_ICLKEN3_CORE:
    {
      value = cm->cmIclkEn3Core;
      break;
    }
    case CM_IDLEST1_CORE:
    {
      value = cm->cmIdleSt1Core;
      break;
    }
    case CM_IDLEST2_CORE:
    {
      value = cm->cmIdleSt2Core;
      break;
    }
    case CM_IDLEST3_CORE:
    {
      value = cm->cmIdleSt3Core;
      break;
    }
    case CM_AUTOIDLE1_CORE:
    {
      value = cm->cmAutoIdle1Core;
      break;
    }
    case CM_AUTOIDLE2_CORE:
    {
      value = cm->cmAutoIdle2Core;
      break;
    }
    case CM_AUTOIDLE3_CORE:
    {
      value = cm->cmAutoIdle3Core;
      break;
    }
    case CM_CLKSEL_CORE:
    {
      value = cm->cmClkSelCore;
      break;
    }
    case CM_CLKSTCTRL_CORE:
    {
      value = cm->cmClkStCtrl;
      break;
    }
    case CM_CLKSTST_CORE:
    {
      value = cm->cmClkStSTCore;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadCoreCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadDssCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - DSS_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_FCLKEN_DSS:
    {
      value = cm->cmFclkEnDss;
      break;
    }
    case CM_ICLKEN_DSS:
    {
      value = cm->cmIclkEnDss;
      break;
    }
    case CM_IDLEST_DSS:
    {
      value = cm->cmIdleStDss;
      break;
    }
    case CM_AUTOIDLE_DSS:
    {
      value = cm->cmAutoIdleDss;
      break;
    }
    case CM_CLKSEL_DSS:
    {
      value = cm->cmClkSelDss;
      break;
    }
    case CM_SLEEPDEP_DSS:
    {
      value = cm->cmSleepDepDss;
      break;
    }
    case CM_CLKSTCTRL_DSS:
    {
      value = cm->cmClkStCtrlDss;
      break;
    }
    case CM_CLKSTST_DSS:
    {
      value = cm->cmClkStStDss;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadDssCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadEmuCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - EMU_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_CLKSEL1_EMU:
    {
      value = cm->cmClkSel1Emu;
      break;
    }
    case CM_CLKSTCTRL_EMU:
    {
      value = cm->cmClkStCtrlEmu;
      break;
    }
    case CM_CLKSTST_EMU:
    {
      value = cm->cmClkStStEmu;
      break;
    }
    case CM_CLKSEL2_EMU:
    {
      value = cm->cmClkSel2Emu;
      break;
    }
    case CM_CLKSEL3_EMU:
    {
      value = cm->cmClkSel3Emu;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadEmuCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadIva2Cm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - IVA2_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_FCLKEN_IVA2:
    {
      value = cm->cmFClkEnIva2Reg;
      break;
    }
    case CM_CLKEN_PLL_IVA2:
    {
      value = cm->cmClkEnPllIva2Reg;
      break;
    }
    case CM_IDLEST_IVA2:
    {
      value = cm->cmIdleStIva2Reg;
      break;
    }
    case CM_IDLEST_PLL_IVA2:
    {
      value = cm->cmIdleStPllIva2Reg;
      break;
    }
    case CM_AUTOIDLE_PLL_IVA2:
    {
      value = cm->cmAutoidlePllIva2Reg;
      break;
    }
    case CM_CLKSEL1_PLL_IVA2:
    {
      value = cm->cmClkSel1PllIva2Reg;
      break;
    }
    case CM_CLKSEL2_PLL_IVA2:
    {
      value = cm->cmClkSel2PllIva2Reg;
      break;
    }
    case CM_CLKSTCTRL_IVA2:
    {
      value = cm->cmClkStCtrlIva2Reg;
      break;
    }
    case CM_CLKSTST_IVA2:
    {
      value = cm->cmClkStStIva2Reg;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadIva2Cm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadMpuCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - MPU_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_CLKEN_PLL_MPU:
    {
      value = cm->cmClkEnPllMpuReg;
      break;
    }
    case CM_IDLEST_MPU:
    {
      value = cm->cmIdleStMpuReg;
      break;
    }
    case CM_IDLEST_PLL_MPU:
    {
      value = cm->cmIdleStPllMpuReg;
      break;
    }
    case CM_AUTOIDLE_PLL_MPU:
    {
      value = cm->cmAutoidlePllMpuReg;
      break;
    }
    case CM_CLKSEL1_PLL_MPU:
    {
      value = cm->cmClkSel1PllMpuReg;
      break;
    }
    case CM_CLKSEL2_PLL_MPU:
    {
      value = cm->cmClkSel2PllMpuReg;
      break;
    }
    case CM_CLKSTCTRL_MPU:
    {
      value = cm->cmClkStCtrlMpuReg;
      break;
    }
    case CM_CLKSTST_MPU:
    {
      value = cm->cmClkStStMpuReg;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadMpuCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadNeonCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - NEON_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_CLKSTCTRL_NEON:
    {
      value = cm->cmClkStCtrlNeon;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadNeonCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadPerCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - PER_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_FCLKEN_PER:
    {
      value = cm->cmFclkEnPer;
      break;
    }
    case CM_ICLKEN_PER:
    {
      value = cm->cmIclkEnPer;
      break;
    }
    case CM_IDLEST_PER:
    {
      value = cm->cmIdleStPer;
      break;
    }
    case CM_AUTOIDLE_PER:
    {
      value = cm->cmAutoIdlePer;
      break;
    }
    case CM_CLKSEL_PER:
    {
      value = cm->cmClkSelPer;
      break;
    }
    case CM_SLEEPDEP_PER:
    {
      value = cm->cmSleepDepPer;
      break;
    }
    case CM_CLKSTCTRL_PER:
    {
      value = cm->cmClkStCtrlPer;
      break;
    }
    case CM_CLKSTST_PER:
    {
      value = cm->cmClkStStPer;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadPerCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadSgxCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - SGX_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_FCLKEN_SGX:
    {
      value = cm->cmFclkEnSgx;
      break;
    }
    case CM_ICLKEN_SGX:
    {
      value = cm->cmIclkEnSgx;
      break;
    }
    case CM_IDLEST_SGX:
    {
      value = cm->cmIdleStSgx;
      break;
    }
    case CM_CLKSEL_SGX:
    {
      value = cm->cmClkSelSgx;
      break;
    }
    case CM_SLEEPDEP_SGX:
    {
      value = cm->cmSleepDepSgx;
      break;
    }
    case CM_CLKSTCTRL_SGX:
    {
      value = cm->cmClkStCtrlSgx;
      break;
    }
    case CM_CLKSTST_SGX:
    {
      value = cm->cmClkStSt;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadSgxCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadUsbHostCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - USBHOST_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_AUTOIDLE_USBHOST:
    {
      value = cm->cmAutoidleUsb;
      break;
    }
    case CM_CLKSTCTRL_USBHOST:
    {
      value = cm->cmClkStCtrlUsb;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadUsbHostCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadWkupCm(struct ClockManager *cm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - WKUP_CM;
  u32int value = 0;
  switch (registerOffset)
  {
    case CM_FCLKEN_WKUP:
    {
      value = cm->cmFclkEnWkup;
      break;
    }
    case CM_ICLKEN_WKUP:
    {
      value = cm->cmIclkEnWkup;
      break;
    }
    case CM_IDLEST_WKUP:
    {
      value = cm->cmIdleStWkup;
      break;
    }
    case CM_AUTOIDLE_WKUP:
    {
      value = cm->cmAutoIdleWkup;
      break;
    }
    case CM_CLKSEL_WKUP:
    {
      value = cm->cmClkSelWkup;
      break;
    }
    case CM_CLKSTCTRL_WKUP:
    {
      DEBUG(VP_OMAP_35XX_CM, "loadWkupCm: undocumented register CM_CLKSTCTRL_WKUP" EOL);
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_CM, "loadWkupCm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

void storeClockManager(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int physicalAddress, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  DEBUG(VP_OMAP_35XX_CM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %#x, val %#.8x" EOL,
        dev->deviceName, physicalAddress, virtAddr, (u32int)size, value);

  ASSERT(size == WORD, ERROR_BAD_ACCESS_SIZE);

  struct ClockManager *const cm = context->vm.clockMan;
  u32int base = physicalAddress & 0xFFFFFF00;
  switch (base)
  {
    case CORE_CM:
    {
      storeCoreCm(cm, physicalAddress, value);
      break;
    }
    case WKUP_CM:
    {
      storeWkupCm(cm, physicalAddress, value);
      break;
    }
    case PER_CM:
    {
      storePerCm(cm, physicalAddress, value);
      break;
    }
    case IVA2_CM:
    {
      storeIva2Cm(cm, physicalAddress, value);
      break;
    }
    case MPU_CM:
    {
      storeMpuCm(cm, physicalAddress, value);
      break;
    }
    case SGX_CM:
    {
      storeSgxCm(cm, physicalAddress, value);
      break;
    }
    case Clock_Control_Reg_CM:
    {
      storeClockControlCm(cm, physicalAddress, value);
      break;
    }
    case DSS_CM:
    {
      storeDssCm(cm, physicalAddress, value);
      break;
    }
    case CAM_CM:
    {
      storeCamCm(cm, physicalAddress, value);
      break;
    }
    case EMU_CM:
    {
      storeEmuCm(cm, physicalAddress, value);
      break;
    }
    case NEON_CM:
    {
      storeNeonCm(cm, physicalAddress, value);
      break;
    }
    case USBHOST_CM:
    {
      storeUsbHostCm(cm, physicalAddress, value);
      break;
    }
    case Global_Reg_CM:
    case OCP_System_Reg_CM:
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    default:
      DIE_NOW(NULL, "invalid base module");
  }
}

static void storeCamCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - CAM_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeCamCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_CLKSTCTRL_CAM:
    {
      if (cm->cmClkStCtrlCam != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkStCtrlCam" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE_CAM:
    {
      if (cm->cmAutoIdleCam != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdleCam" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_FCLKEN_CAM:
    case CM_ICLKEN_CAM:
    case CM_IDLEST_CAM:
    case CM_CLKSEL_CAM:
    case CM_SLEEPDEP_CAM:
    case CM_CLKSTST_CAM:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeClockControlCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - Clock_Control_Reg_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeClockControlCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_CLKEN_PLL:
    {
      printf("Store to CM_CLKEN_PLL val %#.8x" EOL, value);
      cm->cmClkEnPll = value;
      break;
    }
    case CM_CLKEN2_PLL:
    {
      if (cm->cmClkEn2Pll != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkEn2Pll" EOL, __func__);
        if ((value & 0x7) == 0x7)
        {
          cm->cmIdleSt2CkGen |= 0x1;
        }
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE_PLL:
    {
      if (cm->cmAutoIdlePll != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdlePll" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE2_PLL:
    {
      if (cm->cmAutoIdle2Pll != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdle2Pll" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKSEL1_PLL:
      if (cm->cmClkSel1Pll != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkSel1Pll" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    case CM_CLKSEL4_PLL:
    {
      if (cm->cmClkSel4Pll != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkSel4Pll" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_IDLEST_CKGEN:
    case CM_IDLEST2_CKGEN:
    case CM_CLKSEL2_PLL:
    case CM_CLKSEL3_PLL:
    case CM_CLKSEL5_PLL:
    case CM_CLKOUT_CTRL:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeCoreCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - CORE_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeCoreCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_ICLKEN1_CORE:
    {
      const u32int peripherals = cm->cmIclkEn1Core ^ value;
      if (!peripherals)
      {
        break;
      }
      else if (peripherals & CM_CORE_UART1)
      {
        cm->cmIdleSt1Core ^= CM_CORE_UART1;
      }
      else if (peripherals & CM_CORE_SDRC)
      {
        cm->cmIdleSt1Core ^= CM_CORE_SDRC;
      }
      else
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: unimplemented cmIclkEn1Core case %#.8x -> %#.8x" EOL, __func__,
              cm->cmIclkEn1Core, value);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      cm->cmIclkEn1Core = value;
      break;
    }
    case CM_FCLKEN1_CORE:
      if (cm->cmFclkEn1Core != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmFclkEn1Core" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    case CM_CLKSTCTRL_CORE:
      if (cm->cmClkStCtrl != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkStCtrl" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    case CM_AUTOIDLE1_CORE:
    {
      if (cm->cmAutoIdle1Core != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdle1Core" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE2_CORE:
    {
      if (cm->cmAutoIdle2Core != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdle2Core" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE3_CORE:
    {
      if (cm->cmAutoIdle3Core != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdle3Core" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_FCLKEN3_CORE:
    case CM_ICLKEN2_CORE:
    case CM_ICLKEN3_CORE:
    case CM_IDLEST1_CORE:
    case CM_IDLEST2_CORE:
    case CM_IDLEST3_CORE:
    case CM_CLKSEL_CORE:
    case CM_CLKSTST_CORE:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  return;
}

static void storeDssCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - DSS_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeDssCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_ICLKEN_DSS:
    {
      if (cm->cmIclkEnDss != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmIclkEnDss" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKSTCTRL_DSS:
    {
      if (cm->cmClkStCtrlDss != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkstctrlUsb" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE_DSS:
    {
      if (cm->cmAutoIdleDss != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdleDss" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_FCLKEN_DSS:
    case CM_IDLEST_DSS:
    case CM_CLKSEL_DSS:
    case CM_SLEEPDEP_DSS:
    case CM_CLKSTST_DSS:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeEmuCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - EMU_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeEmuCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_CLKSTCTRL_EMU:
    {
      if (cm->cmClkStCtrlEmu != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkStCtrlEmu" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeIva2Cm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - IVA2_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeIva2Cm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_CLKSTCTRL_IVA2:
    {
      if (cm->cmClkStCtrlIva2Reg != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkstctrlIva2" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE_PLL_IVA2:
    {
      if (cm->cmAutoidlePllIva2Reg != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoidlePllIva2Reg" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_FCLKEN_IVA2:
    {
      if (cm->cmFClkEnIva2Reg != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmFClkEnIva2Reg" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKEN_PLL_IVA2:
    case CM_IDLEST_IVA2:
    case CM_IDLEST_PLL_IVA2:
    case CM_CLKSEL1_PLL_IVA2:
    case CM_CLKSEL2_PLL_IVA2:
    case CM_CLKSTST_IVA2:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeMpuCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - MPU_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeMpuCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_CLKSTCTRL_MPU:
    {
      if (cm->cmClkStCtrlMpuReg != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkStCtrlMpu" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_AUTOIDLE_PLL_MPU:
    {
      if (cm->cmAutoidlePllMpuReg != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoidlePllMpuReg" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKEN_PLL_MPU:
    case CM_IDLEST_MPU:
    case CM_IDLEST_PLL_MPU:
    case CM_CLKSEL1_PLL_MPU:
    case CM_CLKSEL2_PLL_MPU:
    case CM_CLKSTST_MPU:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeNeonCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - NEON_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeNeonCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_CLKSTCTRL_NEON:
    {
      if (cm->cmClkStCtrlNeon != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkstctrlNeon" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storePerCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - PER_CM;
  DEBUG(VP_OMAP_35XX_CM, "storePerCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_FCLKEN_PER:
    {
      if (cm->cmFclkEnPer != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmFclkEnPer" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_ICLKEN_PER:
    {
      if (cm->cmIclkEnPer != value)
      {
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      break;
    }
    case CM_IDLEST_PER:
    {
      if (cm->cmIdleStPer != value)
      {
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      break;
    }
    case CM_AUTOIDLE_PER:
    {
      if (cm->cmAutoIdlePer != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdlePer" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKSEL_PER:
    {
      if (cm->cmClkSelPer != value)
      {
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      break;
    }
    case CM_SLEEPDEP_PER:
    {
      if (cm->cmSleepDepPer != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmSleepDepPer" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKSTCTRL_PER:
    {
      if (value & 0x2)
      {
        printf("CMper: WARNING software forced wakeup, deliver interrupt?" EOL);
      }
      cm->cmClkStCtrlPer = value;
      break;
    }
    case CM_CLKSTST_PER:
    {
      if (cm->cmClkStStPer != value)
      {
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      break;
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeSgxCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - SGX_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeSgxCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_CLKSTCTRL_SGX:
    {
      if (cm->cmClkStCtrlSgx != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkstctrlSgx" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeUsbHostCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - USBHOST_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeUsbHostCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_AUTOIDLE_USBHOST:
    {
      if (cm->cmAutoidleUsb != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoidleUsb" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKSTCTRL_USBHOST:
    {
      if (cm->cmClkStCtrlUsb != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmClkstctrlUsb" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_FCLKEN_USBHOST:
    case CM_ICLKEN_USBHOST:
    case CM_IDLEST_USBHOST:
    case CM_SLEEPDEP_USBHOST:
    case CM_CLKSTST_USBHOST:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
}

static void storeWkupCm(struct ClockManager *cm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - WKUP_CM;
  DEBUG(VP_OMAP_35XX_CM, "storeWkupCm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case CM_FCLKEN_WKUP:
    {
      // clear reserved bits... and check meaningful bit values
      value &= ~CM_FCLKEN_WKUP_RESERVED;
      if (cm->cmFclkEnWkup != value)
      {
        if (isUnsettingBits(cm->cmFclkEnWkup, value, CM_FCLKEN_WKUP_WDT2))
        {
          printf("CM: warn: cmWkup disabling watchdog timer 2 functional clock." EOL);
        }
        else if (isSettingBits(cm->cmFclkEnWkup, value, CM_FCLKEN_WKUP_WDT2))
        {
          printf("CM: warn: cmWkup enabling watchdog timer 2 functional clock." EOL);
        }
        if (isUnsettingBits(cm->cmFclkEnWkup, value, CM_FCLKEN_WKUP_GPIO1))
        {
          printf("CM: warn: cmWkup disabling gpio1 functional clock." EOL);
        }
        else if (isSettingBits(cm->cmFclkEnWkup, value, CM_FCLKEN_WKUP_GPIO1))
        {
          printf("CM: warn: cmWkup enabling gpio1 functional clock." EOL);
        }
        if (isUnsettingBits(cm->cmFclkEnWkup, value, CM_FCLKEN_WKUP_ENGPT1))
        {
          printf("CM: warn: cmWkup disabling gptimer1 functional clock." EOL);
        }
        else if (isSettingBits(cm->cmFclkEnWkup, value, CM_FCLKEN_WKUP_ENGPT1))
        {
          printf("CM: warn: cmWkup enabling gptimer1 functional clock." EOL);
        }
        cm->cmFclkEnWkup = value;
      }
      break;
    }
    case CM_ICLKEN_WKUP:
    {
      // clear reserved bits... and check meaningful bit values
      value &= ~CM_ICLKEN_WKUP_RESERVED;
      if (cm->cmIclkEnWkup != value)
      {
        if (isUnsettingBits(cm->cmIclkEnWkup, value, CM_ICLKEN_WKUP_WDT2))
        {
          printf("CM: warn: cmWkup disabling watchdog timer 2 interface clock." EOL);
        }
        else if (isSettingBits(cm->cmIclkEnWkup, value, CM_ICLKEN_WKUP_WDT2))
        {
          printf("CM: warn: cmWkup enabling watchdog timer 2 interface clock." EOL);
        }
        if (isUnsettingBits(cm->cmIclkEnWkup, value, CM_ICLKEN_WKUP_GPIO1))
        {
          printf("CM: warn: cmWkup disabling gpio1 interface clock." EOL);
        }
        else if (isSettingBits(cm->cmIclkEnWkup, value, CM_ICLKEN_WKUP_GPIO1))
        {
          printf("CM: warn: cmWkup enabling gpio1 interface clock." EOL);
        }
        if (isUnsettingBits(cm->cmIclkEnWkup, value, CM_ICLKEN_WKUP_ENGPT1))
        {
          printf("CM: warn: cmWkup disabling gptimer1 interface clock." EOL);
        }
        else if (isSettingBits(cm->cmIclkEnWkup, value, CM_ICLKEN_WKUP_ENGPT1))
        {
          printf("CM: warn: cmWkup enabling gptimer1 interface clock." EOL);
        }
        cm->cmFclkEnWkup = value;
      }
      break;
    }
    case CM_IDLEST_WKUP:
    {
      if (cm->cmIdleStWkup != value)
      {
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      break;
    }
    case CM_AUTOIDLE_WKUP:
    {
      if (cm->cmAutoIdleWkup != value)
      {
#ifdef CONFIG_GUEST_ANDROID
        DEBUG(VP_OMAP_35XX_CM, "%s: ignoring store to cmAutoIdleWkup" EOL, __func__);
#else
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
#endif
      }
      break;
    }
    case CM_CLKSEL_WKUP:
    {
      // clear reserved bits... and check meaningful bit values
      value &= ~CM_CLKSEL_WKUP_RESERVED1;
      if (cm->cmClkSelWkup != value)
      {
        if ((cm->cmClkSelWkup & CM_CLKSEL_WKUP_RM) != (value & CM_CLKSEL_WKUP_RM))
        {
          printf("CM: warn: cmWkup reset module clock set to %x" EOL, (value & CM_CLKSEL_WKUP_RM) >> 1);
        }
        if (isSettingBits(cm->cmIclkEnWkup, value, CM_CLKSEL_WKUP_GPT1))
        {
          printf("CM: warn: cmWkup set gptimer1 clock to system clock" EOL);
        }
        else if (isUnsettingBits(cm->cmIclkEnWkup, value, CM_CLKSEL_WKUP_GPT1))
        {
          printf("CM: warn: cmWkup set gptimer1 clock to 32kHz clock" EOL);
        }
      }
      cm->cmClkSelWkup = value;
      break;
    }
    case CM_CLKSTCTRL_WKUP:
    {
      DEBUG(VP_OMAP_35XX_CM, "storeWkupCm: undocumented register CM_CLKSTCTRL_WKUP" EOL);
      break;
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  } // switch ends
  return;
}
