#ifndef __VM_OMAP_35XX__CLOCK_MANAGER_H__
#define __VM_OMAP_35XX__CLOCK_MANAGER_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


/* CM module instances live at these physical addresses */
#define IVA2_CM                      0x48004000
#define OCP_System_Reg_CM            0x48004800
#define MPU_CM                       0x48004900
#define CORE_CM                      0x48004A00
#define SGX_CM                       0x48004B00
#define WKUP_CM                      0x48004C00
#define Clock_Control_Reg_CM         0x48004D00
#define DSS_CM                       0x48004E00
#define CAM_CM                       0x48004F00
#define PER_CM                       0x48005000
#define EMU_CM                       0x48005100
#define Global_Reg_CM                0x48005200
#define NEON_CM                      0x48005300
#define USBHOST_CM                   0x48005400

/************************************/
/* REGISTER DEFINITIONS and offsets */
/************************************/
// IVA2_CM registers
#define CM_FCLKEN_IVA2              0x00000000 // control IVA2 dom func clock activity, RW 
#define CM_CLKEN_PLL_IVA2           0x00000004 // control IVA2 DPLL modes, RW
#define CM_IDLEST_IVA2              0x00000020 // standby status and avail monitor, R/O
#define CM_IDLEST_PLL_IVA2          0x00000024 // monitor master clock activity, R/O
#define CM_AUTOIDLE_PLL_IVA2        0x00000034 // auto control over IVA2 DPLL activity, RW
#define CM_CLKSEL1_PLL_IVA2         0x00000040 // controls over the IVA2 DPLL, RW
#define CM_CLKSEL2_PLL_IVA2         0x00000044 // controls over the IVA2 DPLL, RW
#define CM_CLKSTCTRL_IVA2           0x00000048 // enables the domain power state transition, RW
#define CM_CLKSTST_IVA2             0x0000004C // status on clock activity in domain, R/O
// OCP_System_Reg_CM registers
#define CM_REVISION                 0x00000000 // IP rev code for the CM part of the PRCM, R/O
#define CM_SYSCONFIG                0x00000010 // various parameters of the interface clock, RW
// MPU_CM registers
#define CM_CLKEN_PLL_MPU            0x00000004 // control DPLL1 modes, RW
#define CM_IDLEST_MPU               0x00000020 // module access availability monitoring, R/O
#define CM_IDLEST_PLL_MPU           0x00000024 // monitoring master clock activity, R/O
#define CM_AUTOIDLE_PLL_MPU         0x00000034 // automatic control over DPLL1, RW
#define CM_CLKSEL1_PLL_MPU          0x00000040 // controls for MPU DPLL, RW
#define CM_CLKSEL2_PLL_MPU          0x00000044 // controls for MPU DPLL, RW
#define CM_CLKSTCTRL_MPU            0x00000048 // enable domain power state transition, RW
#define CM_CLKSTST_MPU              0x0000004C // status of the clock activity in the domain, R/O
// CORE_CM registers
#define CM_FCLKEN1_CORE             0x00000000 // control functional clock activity, RW
#define CM_FCLKEN3_CORE             0x00000008 // control functional clock activity, RW
#define CM_ICLKEN1_CORE             0x00000010 // control interface clock activity, RW
#define CM_ICLKEN2_CORE             0x00000014 // control interface clock activity, RW
#define CM_ICLKEN3_CORE             0x00000018 // control interface clock activity, RW
#define CM_IDLEST1_CORE             0x00000020 // access availability monitoring, R/O
#define CM_IDLEST2_CORE             0x00000024 // access availability monitoring, R/O
#define CM_IDLEST3_CORE             0x00000028 // access availability monitoring, R/O
#define CM_AUTOIDLE1_CORE           0x00000030 // autocontrol of interface clock, RW
#define CM_AUTOIDLE2_CORE           0x00000034 // autocontrol of interface clock, RW
#define CM_AUTOIDLE3_CORE           0x00000038 // autocontrol of interface clock, RW
#define CM_CLKSEL_CORE              0x00000040 // clock selection, RW
#define CM_CLKSTCTRL_CORE           0x00000048 // enable power state transition, RW
#define CM_CLKSTST_CORE             0x0000004C // interface clock activity status, R/O
// SGX_CM registers
#define CM_FCLKEN_SGX               0x00000000 // control graphics engine fclk activity, RW
#define CM_ICLKEN_SGX               0x00000010 // control graphics engine iclk activity, RW
#define CM_IDLEST_SGX               0x00000020 // SGX standby status, R/O
#define CM_CLKSEL_SGX               0x00000040 // SGX clock selection, RW
#define CM_SLEEPDEP_SGX             0x00000044 // en/dis sleep transition depenency, RW
#define CM_CLKSTCTRL_SGX            0x00000048 // enable power state transition, RW  
#define CM_CLKSTST_SGX              0x0000004C // interface clock activity status, R/O
// WKUP_CM registers
#define CM_FCLKEN_WKUP              0x00000000 // control fclk activity, RW
#define CM_FCLKEN_WKUP_RESERVED        0xFFFFFFD6
#define CM_FCLKEN_WKUP_WDT2            0x00000020
#define CM_FCLKEN_WKUP_GPIO1           0x00000008
#define CM_FCLKEN_WKUP_ENGPT1          0x00000001
#define CM_ICLKEN_WKUP              0x00000010 // control iclk activity, RW
#define CM_ICLKEN_WKUP_RESERVED        0xFFFFFFD6
#define CM_ICLKEN_WKUP_WDT2            0x00000020
#define CM_ICLKEN_WKUP_GPIO1           0x00000008
#define CM_ICLKEN_WKUP_ENGPT1          0x00000001
#define CM_IDLEST_WKUP              0x00000020 // access monitoring, R/O
#define CM_AUTOIDLE_WKUP            0x00000030 // autocontrol of iclk activity, RW
#define CM_CLKSEL_WKUP              0x00000040 // source clock selection, RW
#define CM_CLKSEL_WKUP_RESERVED1       0xffffff80
#define CM_CLKSEL_WKUP_RESERVED2       0x00000078
#define CM_CLKSEL_WKUP_RM              0x00000006
#define CM_CLKSEL_WKUP_GPT1            0x00000001
#define CM_CLKSTCTRL_WKUP           0x00000048 // enable power state transition, RW  
// Clock_control_reg_CM registers
#define CM_CLKEN_PLL                0x00000000 // control DPLL3 and DPLL4 modes, RW
#define CM_CLKEN2_PLL               0x00000004 // control DPLL5 modes, RW 
#define CM_UNDOCUMENTED             0x00000010 // undocumented register?
#define CM_IDLEST_CKGEN             0x00000020 // monitor master clock, R/O
#define CM_IDLEST2_CKGEN            0x00000024 // monitor master clock, R/O
#define CM_AUTOIDLE_PLL             0x00000030 // auto control DPLL3 and DPLL4, RW
#define CM_AUTOIDLE2_PLL            0x00000034 // auto control DPLL5, RW
#define CM_CLKSEL1_PLL              0x00000040 // selection of master clock freq, RW
#define CM_CLKSEL2_PLL              0x00000044 // selection of master clock freq, RW
#define CM_CLKSEL3_PLL              0x00000048 // selection of master clock freq, RW
#define CM_CLKSEL4_PLL              0x0000004C // selection of master clock freq, RW
#define CM_CLKSEL5_PLL              0x00000050 // selection of master clock freq, RW
#define CM_CLKOUT_CTRL              0x00000070 // control SYS_CLKOUT2 output clock, RW
// DSS_CM registers
#define CM_FCLKEN_DSS               0x00000000 // control functional clock, RW
#define CM_ICLKEN_DSS               0x00000010 // control interface clock, RW
#define CM_IDLEST_DSS               0x00000020 // access availability monitoring, R/O
#define CM_AUTOIDLE_DSS             0x00000030 // automatic control of interface clk, RW
#define CM_CLKSEL_DSS               0x00000040 // modules clock activity, RW 
#define CM_SLEEPDEP_DSS             0x00000044 // en/dis sleep transition dependency, RW
#define CM_CLKSTCTRL_DSS            0x00000048 // enable domain power state transition, RW
#define CM_CLKSTST_DSS              0x0000004C // OCP interface clock activity status, R/O
// CAM_CM registers
#define CM_FCLKEN_CAM               0x00000000 // control functional clock, RW
#define CM_ICLKEN_CAM               0x00000010 // control interface clock, RW
#define CM_IDLEST_CAM               0x00000020 // access availability monitoring, R/O
#define CM_AUTOIDLE_CAM             0x00000030 // automatic control of interface clk, RW
#define CM_CLKSEL_CAM               0x00000040 // modules clock activity, RW
#define CM_SLEEPDEP_CAM             0x00000044 // en/dis sleep transition dependency, RW
#define CM_CLKSTCTRL_CAM            0x00000048 // enable domain power state transition, RW
#define CM_CLKSTST_CAM              0x0000004C // OCP interface clock activity status, R/O
// PER_CM registers
#define CM_FCLKEN_PER               0x00000000 // control functional clock, RW
#define CM_FCLKEN_PER_RESERVED         0xFFFC0000
#define CM_FCLKEN_PER_GPIO6            0x00020000
#define CM_FCLKEN_PER_GPIO5            0x00010000
#define CM_FCLKEN_PER_GPIO4            0x00008000
#define CM_FCLKEN_PER_GPIO3            0x00004000
#define CM_FCLKEN_PER_GPIO2            0x00002000
#define CM_FCLKEN_PER_WDT3             0x00001000
#define CM_FCLKEN_PER_UART3            0x00000800
#define CM_FCLKEN_PER_GPT9             0x00000400
#define CM_FCLKEN_PER_GPT8             0x00000200
#define CM_FCLKEN_PER_GPT7             0x00000100
#define CM_FCLKEN_PER_GPT6             0x00000080
#define CM_FCLKEN_PER_GPT5             0x00000040
#define CM_FCLKEN_PER_GPT4             0x00000020
#define CM_FCLKEN_PER_GPT3             0x00000010
#define CM_FCLKEN_PER_GPT2             0x00000008
#define CM_FCLKEN_PER_EN_MCBSP4        0x00000004
#define CM_FCLKEN_PER_EN_MCBSP3        0x00000002
#define CM_FCLKEN_PER_EN_MCBSP2        0x00000001
#define CM_ICLKEN_PER               0x00000010 // control interface clock, RW
#define CM_ICLKEN_PER_RESERVED         0xFFFC0000
#define CM_ICLKEN_PER_GPIO6            0x00020000
#define CM_ICLKEN_PER_GPIO5            0x00010000
#define CM_ICLKEN_PER_GPIO4            0x00008000
#define CM_ICLKEN_PER_GPIO3            0x00004000
#define CM_ICLKEN_PER_GPIO2            0x00002000
#define CM_ICLKEN_PER_WDT3             0x00001000
#define CM_ICLKEN_PER_UART3            0x00000800
#define CM_ICLKEN_PER_GPT9             0x00000400
#define CM_ICLKEN_PER_GPT8             0x00000200
#define CM_ICLKEN_PER_GPT7             0x00000100
#define CM_ICLKEN_PER_GPT6             0x00000080
#define CM_ICLKEN_PER_GPT5             0x00000040
#define CM_ICLKEN_PER_GPT4             0x00000020
#define CM_ICLKEN_PER_GPT3             0x00000010
#define CM_ICLKEN_PER_GPT2             0x00000008
#define CM_ICLKEN_PER_EN_MCBSP4        0x00000004
#define CM_ICLKEN_PER_EN_MCBSP3        0x00000002
#define CM_ICLKEN_PER_EN_MCBSP2        0x00000001
#define CM_IDLEST_PER               0x00000020 // access availability monitoring, R/O
#define CM_IDLEST_PER_RESERVED         0xFFFC0000
#define CM_IDLEST_PER_GPIO6            0x00020000
#define CM_IDLEST_PER_GPIO5            0x00010000
#define CM_IDLEST_PER_GPIO4            0x00008000
#define CM_IDLEST_PER_GPIO3            0x00004000
#define CM_IDLEST_PER_GPIO2            0x00002000
#define CM_IDLEST_PER_WDT3             0x00001000
#define CM_IDLEST_PER_UART3            0x00000800
#define CM_IDLEST_PER_GPT9             0x00000400
#define CM_IDLEST_PER_GPT8             0x00000200
#define CM_IDLEST_PER_GPT7             0x00000100
#define CM_IDLEST_PER_GPT6             0x00000080
#define CM_IDLEST_PER_GPT5             0x00000040
#define CM_IDLEST_PER_GPT4             0x00000020
#define CM_IDLEST_PER_GPT3             0x00000010
#define CM_IDLEST_PER_GPT2             0x00000008
#define CM_IDLEST_PER_EN_MCBSP4        0x00000004
#define CM_IDLEST_PER_EN_MCBSP3        0x00000002
#define CM_IDLEST_PER_EN_MCBSP2        0x00000001
#define CM_AUTOIDLE_PER             0x00000030 // automatic control of interface clk, RW
#define CM_AUTOIDLE_PER_RESERVED       0xFFFC0000
#define CM_AUTOIDLE_PER_GPIO6          0x00020000
#define CM_AUTOIDLE_PER_GPIO5          0x00010000
#define CM_AUTOIDLE_PER_GPIO4          0x00008000
#define CM_AUTOIDLE_PER_GPIO3          0x00004000
#define CM_AUTOIDLE_PER_GPIO2          0x00002000
#define CM_AUTOIDLE_PER_WDT3           0x00001000
#define CM_AUTOIDLE_PER_UART3          0x00000800
#define CM_AUTOIDLE_PER_GPT9           0x00000400
#define CM_AUTOIDLE_PER_GPT8           0x00000200
#define CM_AUTOIDLE_PER_GPT7           0x00000100
#define CM_AUTOIDLE_PER_GPT6           0x00000080
#define CM_AUTOIDLE_PER_GPT5           0x00000040
#define CM_AUTOIDLE_PER_GPT4           0x00000020
#define CM_AUTOIDLE_PER_GPT3           0x00000010
#define CM_AUTOIDLE_PER_GPT2           0x00000008
#define CM_AUTOIDLE_PER_EN_MCBSP4      0x00000004
#define CM_AUTOIDLE_PER_EN_MCBSP3      0x00000002
#define CM_AUTOIDLE_PER_EN_MCBSP2      0x00000001
#define CM_CLKSEL_PER               0x00000040 // modules clock activity, RW
#define CM_CLKSEL_PER_RESERVED         0xFFFFFF00
#define CM_CLKSEL_PER_GPT9             0x00000080
#define CM_CLKSEL_PER_GPT8             0x00000040
#define CM_CLKSEL_PER_GPT7             0x00000020
#define CM_CLKSEL_PER_GPT6             0x00000010
#define CM_CLKSEL_PER_GPT5             0x00000008
#define CM_CLKSEL_PER_GPT4             0x00000004
#define CM_CLKSEL_PER_GPT3             0x00000002
#define CM_CLKSEL_PER_GPT2             0x00000001
#define CM_SLEEPDEP_PER             0x00000044 // en/dis sleep transition dependency, RW
#define CM_CLKSTCTRL_PER            0x00000048 // enable domain power state transition, RW
#define CM_CLKSTST_PER              0x0000004C // OCP interface clock activity status, R/O
// EMU_CM registers
#define CM_CLKSEL1_EMU              0x00000040 // modules clock selection, RW
#define CM_CLKSTCTRL_EMU            0x00000048 // en/disable supervised transitions, RW
#define CM_CLKSTST_EMU              0x0000004C // domain clock activity status, R/O 
#define CM_CLKSEL2_EMU              0x00000050 // controls over DPLL3, RW
#define CM_CLKSEL3_EMU              0x00000054 // controls over PERIPHERAL DPLL, RW
// Global_reg_CM registers
#define CM_POLCTRL                  0x0000009C // setting polarity of outputs control signals, RW
// NEON_CM registers
#define CM_IDLEST_NEON              0x00000020 // access availability monitoring, R/O
#define CM_CLKSTCTRL_NEON           0x00000048 // domain clock activity status, R/O
// USBHOST_CM registers
#define CM_FCLKEN_USBHOST           0x00000000 // control functional clock, RW
#define CM_ICLKEN_USBHOST           0x00000010 // control interface clock, RW
#define CM_IDLEST_USBHOST           0x00000020 // access availability monitoring, R/O
#define CM_AUTOIDLE_USBHOST         0x00000030 // automatic control of interface clk, RW
#define CM_SLEEPDEP_USBHOST         0x00000044 // en/dis sleep transition dependency, RW
#define CM_CLKSTCTRL_USBHOST        0x00000048 // enable domain power state transition, RW
#define CM_CLKSTST_USBHOST          0x0000004C // interface clock activity status, R/O


void initClockManager(void);

/* top load function */
u32int loadClockManager(device * dev, ACCESS_SIZE size, u32int address);

u32int loadIva2Cm(device * dev, u32int address, u32int phyAddr);
u32int loadOcpSystemCm(device * dev, u32int address, u32int phyAddr);
u32int loadMpuCm(device * dev, u32int address, u32int phyAddr);
u32int loadCoreCm(device * dev, u32int address, u32int phyAddr);
u32int loadSgxCm(device * dev, u32int address, u32int phyAddr);
u32int loadWkupCm(device * dev, u32int address, u32int phyAddr);
u32int loadClockControlCm(device * dev, u32int address, u32int phyAddr);
u32int loadDssCm(device * dev, u32int address, u32int phyAddr);
u32int loadCamCm(device * dev, u32int address, u32int phyAddr);
u32int loadPerCm(device * dev, u32int address, u32int phyAddr);
u32int loadEmuCm(device * dev, u32int address, u32int phyAddr);
u32int loadGlobalRegCm(device * dev, u32int address, u32int phyAddr);
u32int loadNeonCm(device * dev, u32int address, u32int phyAddr);
u32int loadUsbHostCm(device * dev, u32int address, u32int phyAddr);


/* top store function */
void storeClockManager(device * dev, ACCESS_SIZE size, u32int address, u32int value);

void storeIva2Cm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeOcpSystemCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeMpuCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeCoreCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeSgxCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeWkupCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeClockControlCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeDssCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeCamCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storePerCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeEmuCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeGlobalRegCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeNeonCm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeUsbHostCm(device * dev, u32int address, u32int phyAddr, u32int value);

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
};

#endif
