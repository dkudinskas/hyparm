#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/clockManager.h"
#include "vm/omap35xx/serial.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


extern GCONTXT * getGuestContext(void);

struct ClockManager * clockMan;

void initClockManager()
{
  // init function: setup device, reset register values to defaults!
  clockMan = (struct ClockManager*)mallocBytes(sizeof(struct ClockManager));
  if (clockMan == 0)
  {
    DIE_NOW(0, "Failed to allocate clock manager.");
  }
  else
  {
    memset((void*)clockMan, 0x0, sizeof(struct ClockManager));
#ifdef CLK_MAN_DBG
    serial_putstring("Initializing Clock manager at 0x");
    serial_putint((u32int)clockMan);
    serial_newline();
#endif
  }

  // IVA2_CM registers
  clockMan->cmFClkEnIva2Reg      = 0x00000001; // IVA2_CLK is enabled
  clockMan->cmClkEnPllIva2Reg    = 0x00000037; // enable DPLL in lock mode, 0.75 MHz—1.0 MHz freq
  clockMan->cmIdleStIva2Reg      = 0x00000001; // IVA2 subsystem is in standby
  clockMan->cmIdleStPllIva2Reg   = 0x00000001; // IVA2 DPLL is locked
  clockMan->cmAutoidlePllIva2Reg = 0x00000000; // auto control is disabled
  clockMan->cmClkSel1PllIva2Reg  = 0x0009680c; // div factor 12, multiplier factor 360, DPLL2_FCLK is CORE_CLK divided by 1
  clockMan->cmClkSel2PllIva2Reg  = 0x00000001; // DPLL2 CLKOUTX2 divided by 1
  clockMan->cmClkStCtrlIva2Reg   = 0x00000000; // auto transition is disabled
  clockMan->cmClkStStIva2Reg     = 0x00000001; // domain clock is active
  // MPU_CM registers
  clockMan->cmClkEnPllMpuReg    = 0x00000037; // enable DPLL in lock mode, 0.75 MHz—1.0 MHz freq
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
  clockMan->cmClkEnPll    = 0x00370037; // DPLL3, DPLL4 in lock mode @ 0.75 MHz—1.0 MHz
  clockMan->cmClkEn2Pll   = 0x00000011; // DPLL5 in low power stop mode @ 0.75 MHz—1.0 MHz
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
u32int loadClockManager(device * dev, ACCESS_SIZE size, u32int address)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef CLK_MAN_DBG
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
    DIE_NOW(0, "CM: invalid access size.");
  }

  u32int val = 0;
  
  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case IVA2_CM:
      val = loadIva2Cm(dev, address, phyAddr);
      break;
    case OCP_System_Reg_CM:
      val = loadOcpSystemCm(dev, address, phyAddr);
      break;
    case MPU_CM:
      val = loadMpuCm(dev, address, phyAddr);
      break;
    case CORE_CM:
      val = loadCoreCm(dev, address, phyAddr);
      break;
    case SGX_CM:
      val = loadSgxCm(dev, address, phyAddr);
      break;
    case WKUP_CM:
      val = loadWkupCm(dev, address, phyAddr);
      break;
    case Clock_Control_Reg_CM:
      val = loadClockControlCm(dev, address, phyAddr);
      break;
    case DSS_CM:
      val = loadDssCm(dev, address, phyAddr);
      break;
    case CAM_CM:
      val = loadCamCm(dev, address, phyAddr);
      break;
    case PER_CM:
      val = loadPerCm(dev, address, phyAddr);
      break;
    case EMU_CM:
      val = loadEmuCm(dev, address, phyAddr);
      break;
    case Global_Reg_CM:
      val = loadGlobalRegCm(dev, address, phyAddr);
      break;
    case NEON_CM:
      val = loadNeonCm(dev, address, phyAddr);
      break;
    case USBHOST_CM:
      val = loadUsbHostCm(dev, address, phyAddr);
      break;
    default:
      DIE_NOW(0, "CM: invalid base module.");
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadIva2Cm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadIva2Cm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
  return val;
}


u32int loadOcpSystemCm(device * dev, u32int address, u32int phyAddr)
{
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_newline();
  serial_putstring(dev->deviceName);
  dumpGuestContext(getGuestContext());
  DIE_NOW(0, " loadOcpSystemCm unimplemented.");
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadMpuCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadMpuCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadCoreCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadCoreCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadSgxCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadSgxCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
#ifdef CLK_MAN_DBG
      serial_putstring("CLKMAN: warn: wkupCm load from undocumented reg (offs 0x48)");
#endif  
      val = 0;
      break;
    default:
      serial_putstring("loadWkupCm reg ");
      serial_putint_nozeros(reg);
      serial_newline();
      dumpGuestContext(getGuestContext());      
      DIE_NOW(0, "loadWkupCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadWkupCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
#ifdef CLK_MAN_DBG
      serial_putstring("loadWkupCm warning: loading from undocumented reg ");
      serial_putint_nozeros(reg);
      serial_putstring(" returning zero");
      serial_newline(); 
#endif
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
      dumpGuestContext(getGuestContext());
      serial_putstring("loadClockControlCm reg ");
      serial_putint_nozeros(reg);
      serial_newline();
      DIE_NOW(0, "loadClockControlCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadClockControlCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadDssCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadDssCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadCamCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadCamCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadPerCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadPerCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
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
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "loadEmuCm loading non existing register!");
  } // switch ends
#ifdef CLK_MAN_DBG
  serial_putstring("loadEmuCm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
  return val;
}

u32int loadGlobalRegCm(device * dev, u32int address, u32int phyAddr)
{
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_newline();
  serial_putstring(dev->deviceName);
  serial_newline();
  dumpGuestContext(getGuestContext());
  DIE_NOW(0, " loadGlobalRegCm unimplemented.");
  return 0;
}


u32int loadNeonCm(device * dev, u32int address, u32int phyAddr)
{
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_newline();
  serial_putstring(dev->deviceName);
  dumpGuestContext(getGuestContext());
  DIE_NOW(0, " loadNeonCm unimplemented.");
  return 0;
}


u32int loadUsbHostCm(device * dev, u32int address, u32int phyAddr)
{
  serial_putstring(dev->deviceName);
  serial_putstring(" load from pAddr: 0x");
  serial_putint(phyAddr);
  serial_putstring(", vAddr: 0x");
  serial_putint(address);
  serial_newline();
  serial_putstring(dev->deviceName);
  dumpGuestContext(getGuestContext());
  DIE_NOW(0, " loadUsbHostCm unimplemented.");
  return 0;
}


/*************************************************************************
 *                           Store Functions                             *
 *************************************************************************/
void storeClockManager(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef CLK_MAN_DBG
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

  
  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case CORE_CM:
      storeCoreCm(dev, address, phyAddr, value);
      break;
    case WKUP_CM:
      storeWkupCm(dev, address, phyAddr, value);
      break;
    case PER_CM:
      storePerCm(dev, address, phyAddr, value);
      break;
    case IVA2_CM:
      storeIva2Cm(dev, address, phyAddr, value);
      break;
    case OCP_System_Reg_CM:
      storeOcpSystemCm(dev, address, phyAddr, value);
      break;
    case MPU_CM:
      storeMpuCm(dev, address, phyAddr, value);
      break;
    case SGX_CM:
      storeSgxCm(dev, address, phyAddr, value);
      break;
    case Clock_Control_Reg_CM:
      storeClockControlCm(dev, address, phyAddr, value);
      break;
    case DSS_CM:
      storeDssCm(dev, address, phyAddr, value);
      break;
    case CAM_CM:
      storeCamCm(dev, address, phyAddr, value);
      break;
    case EMU_CM:
      storeEmuCm(dev, address, phyAddr, value);
      break;
    case Global_Reg_CM:
      storeGlobalRegCm(dev, address, phyAddr, value);
      break;
    case NEON_CM:
      storeNeonCm(dev, address, phyAddr, value);
      break;
    case USBHOST_CM:
      storeUsbHostCm(dev, address, phyAddr, value);
      break;
    default:
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "CM: store to invalid base module.");
  } // switch ends
} // storeClockManager


void storeIva2Cm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  serial_newline();
  DIE_NOW(0, " storeIva2Cm unimplemented.");
  return;
}


void storeOcpSystemCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeOcpSystemCm unimplemented.");
  return;
}


void storeMpuCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeMpuCm unimplemented.");
  return;
}


void storeCoreCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
#ifdef CLK_MAN_DBG
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
#endif
  u32int reg = phyAddr - CORE_CM;
  switch (reg)
  {
    case CM_ICLKEN1_CORE:
      if (clockMan->cmIclkEn1Core != value)
      {
        DIE_NOW(0, " storeCoreCm CM_ICLKEN1_CORE unimplemented.");
      }
      break;
    case CM_FCLKEN1_CORE:
      if (clockMan->cmFclkEn1Core != value)
      {
        DIE_NOW(0, " storeCoreCm CM_FCLKEN1_CORE unimplemented.");
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
      DIE_NOW(0, " storeCoreCm unimplemented.");
      break;
    default:
      DIE_NOW(0, "storeCoreCm storing non existing register!");
  } // switch ends
  return;
}


void storeSgxCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeSgxCm unimplemented.");
  return;
}


void storeWkupCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
#ifdef CLK_MAN_DBG
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
#endif
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
          serial_putstring("CM: warn: cmWkup disabling watchdog timer 2 functional clock.");
          serial_newline();
        }
        else if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_WDT2) == 0) &&
             ((value & CM_FCLKEN_WKUP_WDT2) == CM_FCLKEN_WKUP_WDT2) )
        {
          serial_putstring("CM: warn: cmWkup enabling watchdog timer 2 functional clock.");
          serial_newline();
        }
        if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_GPIO1) == CM_FCLKEN_WKUP_GPIO1) &&
             ((value & CM_FCLKEN_WKUP_GPIO1) == 0) )
        {
          serial_putstring("CM: warn: cmWkup disabling gpio1 functional clock.");
          serial_newline();
        }
        else if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_GPIO1) == 0) &&
             ((value & CM_FCLKEN_WKUP_GPIO1) == CM_FCLKEN_WKUP_GPIO1) )
        {
          serial_putstring("CM: warn: cmWkup enabling gpio1 functional clock.");
          serial_newline();
        }
        if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_ENGPT1) == CM_FCLKEN_WKUP_ENGPT1) &&
             ((value & CM_FCLKEN_WKUP_ENGPT1) == 0) )
        {
          serial_putstring("CM: warn: cmWkup disabling gptimer1 functional clock.");
          serial_newline();
        }
        else if ( ((clockMan->cmFclkEnWkup & CM_FCLKEN_WKUP_ENGPT1) == 0) &&
             ((value & CM_FCLKEN_WKUP_ENGPT1) == CM_FCLKEN_WKUP_ENGPT1) )
        {
          serial_putstring("CM: warn: cmWkup enabling gptimer1 functional clock.");
          serial_newline();
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
          serial_putstring("CM: warn: cmWkup disabling watchdog timer 2 interface clock.");
          serial_newline();
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_WDT2) == 0) &&
             ((value & CM_ICLKEN_WKUP_WDT2) == CM_ICLKEN_WKUP_WDT2) )
        {
          serial_putstring("CM: warn: cmWkup enabling watchdog timer 2 interface clock.");
          serial_newline();
        }
        if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_GPIO1) == CM_ICLKEN_WKUP_GPIO1) &&
             ((value & CM_ICLKEN_WKUP_GPIO1) == 0) )
        {
          serial_putstring("CM: warn: cmWkup disabling gpio1 interface clock.");
          serial_newline();
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_GPIO1) == 0) &&
             ((value & CM_ICLKEN_WKUP_GPIO1) == CM_ICLKEN_WKUP_GPIO1) )
        {
          serial_putstring("CM: warn: cmWkup enabling gpio1 interface clock.");
          serial_newline();
        }
        if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_ENGPT1) == CM_ICLKEN_WKUP_ENGPT1) &&
             ((value & CM_ICLKEN_WKUP_ENGPT1) == 0) )
        {
          serial_putstring("CM: warn: cmWkup disabling gptimer1 interface clock.");
          serial_newline();
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_ICLKEN_WKUP_ENGPT1) == 0) &&
             ((value & CM_ICLKEN_WKUP_ENGPT1) == CM_ICLKEN_WKUP_ENGPT1) )
        {
          serial_putstring("CM: warn: cmWkup enabling gptimer1 interface clock.");
          serial_newline();
        }
        clockMan->cmFclkEnWkup = value;
      }
      break;
    case CM_IDLEST_WKUP:
      if (clockMan->cmIdleStWkup != value)
      {
        DIE_NOW(0, " storeWkupCm unimplemented store to reg cmIdleStWkup");
      }
      break;
    case CM_AUTOIDLE_WKUP:
      if (clockMan->cmAutoIdleWkup != value)
      {
        DIE_NOW(0, " storeWkupCm unimplemented store to reg cmAutoIdleWkup");
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
          serial_putstring("CM: warn: cmWkup reset module clock set to ");
          serial_putint_nozeros((value & CM_CLKSEL_WKUP_RM) >> 1);
          serial_newline();
        }
        if ( ((clockMan->cmClkSelWkup & CM_CLKSEL_WKUP_GPT1) == 0) &&
             ((value & CM_CLKSEL_WKUP_GPT1) == CM_CLKSEL_WKUP_GPT1) )
        {
          serial_putstring("CM: warn: cmWkup set gptimer1 clock to system clock");
          serial_newline();
        }
        else if ( ((clockMan->cmIclkEnWkup & CM_CLKSEL_WKUP_GPT1) == CM_CLKSEL_WKUP_GPT1) &&
             ((value & CM_CLKSEL_WKUP_GPT1) == 0) )
        {
          serial_putstring("CM: warn: cmWkup set gptimer1 clock to 32kHz clock");
          serial_newline();
        }
      }
      clockMan->cmClkSelWkup = value;
      break;
    case CM_CLKSTCTRL_WKUP:
#ifdef CLK_MAN_DBG
      serial_putstring(dev->deviceName);
      serial_putstring(" WARN: store to undocumented register CM_CLKSTCTRL_WKUP");
      serial_newline();
#endif
      break;
    default:
      DIE_NOW(0, "storeWkupCm storing non existing register!");
  } // switch ends
  return;
}


void storeClockControlCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
#ifdef CLK_MAN_DBG
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at VA ");
  serial_putint(address);
  serial_putstring(" at PA ");
  serial_putint(phyAddr);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
#endif

  u32int reg = phyAddr - Clock_Control_Reg_CM;
  switch (reg)
  {
    case CM_CLKEN_PLL:
      serial_putstring("Store to CM_CLKEN_PLL val ");
      serial_putint(value);
      serial_newline();
      clockMan->cmClkEnPll = value;
      break;
    case CM_AUTOIDLE_PLL:
      serial_putstring("Store to CM_AUTOIDLE_PLL val ");
      serial_putint(value);
      serial_newline();
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
      serial_putstring("Store to: ");
      serial_putstring(dev->deviceName);
      serial_putstring(" at VA ");
      serial_putint(address);
      serial_putstring(" at PA ");
      serial_putint(phyAddr);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "storeClockControlCm store to unimplemented register register!");
      break;
    default:
      serial_putstring("Store to: ");
      serial_putstring(dev->deviceName);
      serial_putstring(" at VA ");
      serial_putint(address);
      serial_putstring(" at PA ");
      serial_putint(phyAddr);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      dumpGuestContext(getGuestContext());
      DIE_NOW(0, "storeClockControlCm storing non existing register!");
  } // switch ends
}

void storeDssCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeDssCm unimplemented.");
  return;
}


void storeCamCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeCamCm unimplemented.");
  return;
}


void storePerCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
#ifdef CLK_MAN_DBG
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
#endif
  u32int reg = phyAddr - PER_CM;
  switch (reg)
  {
    case CM_FCLKEN_PER:
      if (clockMan->cmFclkEnPer != value)
      {
        DIE_NOW(0, " storePerCm unimplemented store to reg cmFclkEnPer");
      }
      break;
    case CM_ICLKEN_PER:
      if (clockMan->cmIclkEnPer != value)
      {
        DIE_NOW(0, " storePerCm unimplemented store to reg cmIclkEnPer");
      }
      break;
    case CM_IDLEST_PER:
      if (clockMan->cmIdleStPer != value)
      {
        DIE_NOW(0, " storePerCm unimplemented store to reg cmIdleStPer");
      }
      break;
    case CM_AUTOIDLE_PER:
      if (clockMan->cmAutoIdlePer != value)
      {
        DIE_NOW(0, " storePerCm unimplemented store to reg cmAutoIdlePer");
      }
      break;
    case CM_CLKSEL_PER:
      if (clockMan->cmClkSelPer != value)
      {
        DIE_NOW(0, " storePerCm unimplemented store to reg cmClkSelPer");
      }
      break;
    case CM_SLEEPDEP_PER:
      if (clockMan->cmSleepDepPer != value)
      {
        DIE_NOW(0, " storePerCm unimplemented store to reg cmSleepDepPer");
      }
      break;
    case CM_CLKSTCTRL_PER:
      if (value & 0x2)
      {
        serial_putstring("CMper: WARNING software forced wakeup, deliver interrupt?");
        serial_newline();
      }
      clockMan->cmClkStCtrlPer = value;
      break;
    case CM_CLKSTST_PER:
      if (clockMan->cmClkStStPer != value)
      {
        DIE_NOW(0, " storePerCm unimplemented store to reg cmClkStStPer");
      }
      break;
    default:
      DIE_NOW(0, "storePerCm storing non existing register!");
  } // switch ends
}


void storeEmuCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeEmuCm unimplemented.");
  return;
}

void storeGlobalRegCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  dumpGuestContext(getGuestContext());
  DIE_NOW(0, " storeGlobalRegCm unimplemented.");
  return;
}


void storeNeonCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  dumpGuestContext(getGuestContext());
  DIE_NOW(0, " storeNeonCm unimplemented.");
  return;
}


void storeUsbHostCm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  dumpGuestContext(getGuestContext());
  DIE_NOW(0, " storeUsbHostCm unimplemented.");
  return;
}
