#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/prm.h"


/* PRM module instances live at these physical addresses */
#define IVA2_PRM                      0x48306000
#define OCP_System_Reg_PRM            0x48306800
#define MPU_PRM                       0x48306900
#define CORE_PRM                      0x48306A00
#define SGX_PRM                       0x48306B00
#define WKUP_PRM                      0x48306C00
#define Clock_Control_Reg_PRM         0x48306D00
#define DSS_PRM                       0x48306E00
#define CAM_PRM                       0x48306F00
#define PER_PRM                       0x48307000
#define EMU_PRM                       0x48307100
#define Global_Reg_PRM                0x48307200
#define NEON_PRM                      0x48307300
#define USBHOST_PRM                   0x48307400


/************************************/
/* REGISTER DEFINITIONS and offsets */
/************************************/
// COMMON REGISTER OFFSETS
#define RM_RSTCTRL               0x00000050 // reset control, RW
#define RM_RSTST                 0x00000058 // reset status, RW
#define PM_WKEN                  0x000000A0 // wakeup events enable, RW
#define PM_MPUGRPSEL             0x000000A4 // select group of modules that wakeup MPU, RW
#define PM_IVA2GRPSEL            0x000000A8 // select group of modules that wakeup IVA, RW
#define PM_WKST                  0x000000B0 // wakeup events status, RW
#define PM_WKDEP                 0x000000C8 // wakeup disable / enable, RW
#define PM_PWSTCTRL              0x000000E0 // power state transition, RW
#define PM_PWSTST                0x000000E4 // power state status, R/O
#define PM_PREPWSTST             0x000000E8 // previous power state status, RW

// MODULE SPECIFIC OFFSETS
// IVA2 register offsets
#define PRM_IRQSTATUS_IVA2       0x000000F8 // IRQ status, RW
#define PRM_IRQENABLE_IVA2       0x000000FC // IRQ enable, RW
// OCP_system_reg register offsets
#define PRM_REVISION_OCP           0x00000004 // revision number, R/O
#define PRM_SYSCONFIG_OCP          0x00000014 // various parameters for the i-face, RW
#define PRM_SYSCONFIG_OCP_RESERVED 0xFFFFFFFE
#define PRM_SYSCONFIG_OCP_AUTOIDLE 0x00000001
#define PRM_IRQSTATUS_MPU_OCP      0x00000018 // IRQ status, RW
#define PRM_IRQENABLE_MPU_OCP      0x0000001C // IRQ enable, RW
// MPU register offsets
#define PM_EVGENCTRL_MPU         0x000000D4 // event generator control, RW
#define PM_EVGENONTIM_MPU        0x000000D8 // sets the ON count duration of the gen., RW
#define PM_EVGENOFFTIM_MPU       0x000000DC // sets the OFF count duration of the gen., RW
// CORE register offsets
#define PM_WKST3_CORE            0x000000B8 // wakeup events status, RW
#define PM_WKEN3_CORE            0x000000F0 // wakeup enable, RW
#define PM_IVA2GRPSEL3_CORE      0x000000F4 // select group of modules that wakeup MPU, RW
#define PM_MPUGRPSEL3_CORE       0x000000F8 // select group of modules that wakeup IVA2, RW
// Clock_Control_Reg register offsets
#define PRM_CLKSEL               0x00000040 // select system clock frequency, RW
#define PRM_CLKOUT_CTRL          0x00000070 // SYS_CLKOUT1 pin control, RW
// Global_Reg register offsets
#define PRM_VC_SMPS_SA           0x00000020 // setting I2C slave addR of the Power IC, RW
#define PRM_VC_SMPS_VOL_RA       0x00000024 // setting voltage conf reg address for VDD channels, RW
#define PRM_VC_SMPS_CMD_RA       0x00000028 // setting ON/Retention/OFF cmnd conf reg addr for VDD, RW
#define PRM_VC_CMD_VAL_0         0x0000002C // ON/Retention/OFF voltage level values for 1st VDD, RW
#define PRM_VC_CMD_VAL_1         0x00000030 // ON/Retention/OFF voltage level values for 2nd VDD, RW
#define PRM_VC_CH_CONF           0x00000034 // config pointers for both VDD channels, RW
#define PRM_VC_I2C_CFG           0x00000038 // config pointers for both VDD channels, RW
#define PRM_VC_BYPASS_VAL        0x0000003C // programming the PowerIC dev using bypass i-face, RW
#define PRM_RSTCTRL              0x00000050 // Global software and DPLL3 reset control, RW
#define PRM_RSTTIME              0x00000054 // Reset duration control, RW
#define PRM_RSTST                0x00000058 // reset status, RW
#define PRM_VOLTCTRL             0x00000060 // direct control on the external power IC, RW
#define PRM_SRAM_PCHARGE         0x00000064 // setting the pre-charge time of the SRAM, RW
#define PRM_CLKSRC_CTRL          0x00000070 // control over the device source clock, RW
#define PRM_OBSR                 0x00000080 // logs observable signals, R/O
#define PRM_VOLTSETUP1           0x00000090 // setting setup time of VDD1 & VDD2 regulators, RW
#define PRM_VOLTOFFSET           0x00000094 // controlling the sys_offmode signal upon wake-up, RW
#define PRM_CLKSETUP             0x00000098 // setup time of the oscillator system clock (sys_clk), RW
#define PRM_POLCTRL              0x0000009C // polarity of device outputs control signals.
#define PRM_VOLTSETUP2           0x000000A0 // overall setup time of VDD1 and VDD2 regulators, RW

/*
 * UNKNOWN REGISTER OFFSET
 * This uknown register offset is used by some linux kernels.
 * It tries to to read-modify-write this registers
 *   which belong to no module.
 */
#define PM_UNKNOWN               0x00000044 // unknown register


/* per-module loads */
static u32int loadCamPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadClockControlPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadCorePrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadDssPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadEmuPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadGlobalRegPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadIva2Prm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadMpuPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadNeonPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadOcpSystemPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadPerPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadSgxPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadUsbhostPrm(struct PowerAndResetManager *prm, u32int physicalAddress);
static u32int loadWakeUpPrm(struct PowerAndResetManager *prm, u32int physicalAddress);

/* per-module stores */
static void storeCamPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeClockControlPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeCorePrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeDssPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeEmuPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeIva2Prm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeMpuPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeNeonPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeOcpSystemPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storePerPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeSgxPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeUsbhostPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeWakeUpPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);


void initPrm(virtualMachine *vm)
{
  // init function: setup device, reset register values to defaults!
  struct PowerAndResetManager *prm = (struct PowerAndResetManager *)calloc(1, sizeof(struct PowerAndResetManager));
  if (prm == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate power and reset manager.");
  }

  DEBUG(VP_OMAP_35XX_PRM, "Initializing Power and reset manager at %p" EOL, prm);

  // Clock Control registers
  prm->prmClkSelReg = 0x3;
  prm->prmClkoutCtrlReg = 0x80;
  // Global reg prm registers
  prm->prmVcSmpsSa = 0;
  prm->prmVcSmpsVolRa = 0;
  prm->prmVcSmpsCmdRa = 0;
  prm->prmVcCmdVal0 = 0;
  prm->prmVcCmdVal1 = 0;
  prm->prmVcChConf = 0;
  prm->prmVcI2cCfg = 0x18; // ([3]I2C bus high speed mode, [4] repeated start operation mode)
  prm->prmVcBypassVal = 0;
  prm->prmRstCtrl = 0;
  prm->prmRstTime = 0x1006; // [12:8] rstTime2, [0:7] rstTime1
  prm->prmRstState = 0x1;
  prm->prmVoltCtrl = 0;
  prm->prmSramPcharge = 0x50; // sram precharge duration in clk cycles
  prm->prmClkSrcCtrl = 0x80;
  prm->prmObsr = 0;
  prm->prmVoltSetup1 = 0;
  prm->prmVoltOffset = 0;
  prm->prmClkSetup = 0;
  prm->prmPolCtrl = 0xA;
  prm->prmVoltSetup2 = 0;
  // IVA2 registers
  prm->prmWkdepIva2    = 0xB3;
  prm->prmPwstctrlIva2 = 0xFF0F07;
  prm->prmPwststIva2   = 0xFF7;
  // OCP_system_reg REGISTERS
  prm->prmRevisionOcp     = 0x10;
  prm->prmSysConfigOcp    = 0x1;
  prm->prmIrqStatusMpuOcp = 0x0;
  prm->prmIrqEnableMpuOcp = 0x0;
  // MPU registers
  prm->prmWkdepMpu    = 0xA5;
  prm->prmPwstctrlMpu = 0x30107;
  prm->prmPwststMpu   = 0xC7;
  // CORE registers
  prm->prmPwstctrlCore = 0xF0307;
  prm->prmPwststCore   = 0xF7;
  // SGX registers
  prm->prmWkdepSgx    = 0x16;
  prm->prmPwstctrlSgx = 0x30107;
  prm->prmPwststSgx   = 0x3;
  // Wakeup registers
  prm->prmWkenWkup       = 0x3CB;
  prm->prmMpugrpselWkup  = 0x3CB;
  prm->prmIva2grpselWkup = 0;
  prm->prmWkstWkup       = 0;
  // DSS registers
  prm->prmWkdepDss    = 0x16;
  prm->prmPwstctrlDss = 0x30107;
  prm->prmPwststDss   = 0x3;
  // CAM registers
  prm->prmWkdepCam    = 0x16;
  prm->prmPwstctrlCam = 0x30107;
  prm->prmPwststCam   = 0x3;
  // PER registers
  prm->prmPwstctrlPer = 0x30107;
  prm->prmPwststPer   = 0x7;
  // EMU registers
  prm->prmPwststEmu = 0xC3;
  // NEON registers
  prm->prmWkdepNeon    = 0x2;
  prm->prmPwstctrlNeon = 0x7;
  prm->prmPwststNeon   = 0x3;
  // USBHOST registers
  prm->prmWkdepUsbhost    = 0x17;
  prm->prmPwstctrlUsbhost = 0x30107;
  prm->prmPwststUsbhost   = 0x3;

  vm->prMan = prm;
}

u32int loadPrm(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  DEBUG(VP_OMAP_35XX_PRM, "%s load from physical address: %#.8x, vAddr %#.8x, aSize %x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  ASSERT(size == WORD, ERROR_BAD_ACCESS_SIZE);

  struct PowerAndResetManager *const prm = context->vm.prMan;
  const u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case Clock_Control_Reg_PRM:
      return loadClockControlPrm(prm, phyAddr);
    case Global_Reg_PRM:
      return loadGlobalRegPrm(prm, phyAddr);
    case IVA2_PRM:
      return loadIva2Prm(prm, phyAddr);
    case OCP_System_Reg_PRM:
      return loadOcpSystemPrm(prm, phyAddr);
    case MPU_PRM:
      return loadMpuPrm(prm, phyAddr);
    case CORE_PRM:
      return loadCorePrm(prm, phyAddr);
    case SGX_PRM:
      return loadSgxPrm(prm, phyAddr);
    case WKUP_PRM:
      return loadWakeUpPrm(prm, phyAddr);
    case DSS_PRM:
      return loadDssPrm(prm, phyAddr);
    case CAM_PRM:
      return loadCamPrm(prm, phyAddr);
    case PER_PRM:
      return loadPerPrm(prm, phyAddr);
    case EMU_PRM:
      return loadEmuPrm(prm, phyAddr);
    case NEON_PRM:
      return loadNeonPrm(prm, phyAddr);
    case USBHOST_PRM:
      return loadUsbhostPrm(prm, phyAddr);
    default:
    {
      printf("virtual address %#.8x physical address %#.8x" EOL, virtAddr, phyAddr);
      DIE_NOW(NULL, "invalid base module");
    }
  }
}

static u32int loadCamPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - CAM_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      value = prm->prmWkdepCam;
      break;
    }
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlCam;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststCam;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadCamPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadClockControlPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - Clock_Control_Reg_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PRM_CLKSEL:
    {
      value = prm->prmClkSelReg;
      break;
    }
    case PRM_CLKOUT_CTRL:
    {
      value = prm->prmClkoutCtrlReg;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_PWSTST:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadClockControlPrm reg %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadCorePrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - CORE_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlCore;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststCore;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadCorePrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadDssPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - DSS_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      value = prm->prmWkdepDss;
      break;
    }
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlDss;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststDss;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadDssPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadEmuPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - EMU_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_PWSTST:
    {
      value = prm->prmPwststEmu;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadEmuPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadGlobalRegPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - Global_Reg_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PRM_VC_SMPS_SA:
    {
      value = prm->prmVcSmpsSa;
      break;
    }
    case PRM_VC_SMPS_VOL_RA:
    {
      value = prm->prmVcSmpsVolRa;
      break;
    }
    case PRM_VC_SMPS_CMD_RA:
    {
      value = prm->prmVcSmpsCmdRa;
      break;
    }
    case PRM_VC_CMD_VAL_0:
    {
      value = prm->prmVcCmdVal0;
      break;
    }
    case PRM_VC_CMD_VAL_1:
    {
      value = prm->prmVcCmdVal1;
      break;
    }
    case PRM_VC_CH_CONF:
    {
      value = prm->prmVcChConf;
      break;
    }
    case PRM_VC_I2C_CFG:
    {
      value = prm->prmVcI2cCfg;
      break;
    }
    case PRM_VC_BYPASS_VAL:
    {
      value = prm->prmVcBypassVal;
      break;
    }
    case PRM_RSTCTRL:
    {
      value = prm->prmRstCtrl;
      break;
    }
    case PRM_RSTTIME:
    {
      value = prm->prmRstTime;
      break;
    }
    case PRM_RSTST:
    {
      value = prm->prmRstState;
      break;
    }
    case PRM_VOLTCTRL:
    {
      value = prm->prmVoltCtrl;
      break;
    }
    case PRM_SRAM_PCHARGE:
    {
      value = prm->prmSramPcharge;
      break;
    }
    case PRM_CLKSRC_CTRL:
    {
      value = prm->prmClkSrcCtrl;
      break;
    }
    case PRM_OBSR:
    {
      value = prm->prmObsr;
      break;
    }
    case PRM_VOLTSETUP1:
    {
      value = prm->prmVoltSetup1;
      break;
    }
    case PRM_VOLTOFFSET:
    {
      value = prm->prmVoltOffset;
      break;
    }
    case PRM_CLKSETUP:
    {
      value = prm->prmClkSetup;
      break;
    }
    case PRM_POLCTRL:
    {
      value = prm->prmPolCtrl;
      break;
    }
    case PRM_VOLTSETUP2:
    {
      value = prm->prmVoltSetup2;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadGlobalRegPrm reg %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadIva2Prm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - IVA2_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      value = prm->prmWkdepIva2;
      break;
    }
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlIva2;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststIva2;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    case RM_RSTCTRL:
    case RM_RSTST:
    case PM_PREPWSTST:
    case PRM_IRQSTATUS_IVA2:
    case PRM_IRQENABLE_IVA2:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadIva2Prm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadMpuPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - MPU_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      value = prm->prmWkdepMpu;
      break;
    }
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlMpu;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststMpu;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    case RM_RSTST:
    case PM_EVGENCTRL_MPU:
    case PM_EVGENONTIM_MPU:
    case PM_EVGENOFFTIM_MPU:
    case PM_PREPWSTST:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadMpuPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadNeonPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - NEON_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      value = prm->prmWkdepNeon;
      break;
    }
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlNeon;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststNeon;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadNeonPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadOcpSystemPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - OCP_System_Reg_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PRM_REVISION_OCP:
    {
      value = prm->prmRevisionOcp;
      break;
    }
    case PRM_SYSCONFIG_OCP:
    {
      value = prm->prmSysConfigOcp;
      break;
    }
    case PRM_IRQSTATUS_MPU_OCP:
    {
      value = prm->prmIrqStatusMpuOcp;
      break;
    }
    case PRM_IRQENABLE_MPU_OCP:
    {
      value = prm->prmIrqEnableMpuOcp;
      break;
    }
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadOcpSystemPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadPerPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - PER_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlPer;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststPer;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadPerPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadSgxPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - SGX_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlSgx;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststSgx;
      break;
    }
    case PM_WKDEP:
    {
      value = prm->prmWkdepSgx;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadSgxPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadUsbhostPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - USBHOST_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      value = prm->prmWkdepUsbhost;
      break;
    }
    case PM_PWSTCTRL:
    {
      value = prm->prmPwstctrlUsbhost;
      break;
    }
    case PM_PWSTST:
    {
      value = prm->prmPwststUsbhost;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadUsbhostPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

static u32int loadWakeUpPrm(struct PowerAndResetManager *prm, u32int physicalAddress)
{
  const u32int registerOffset = physicalAddress - WKUP_PRM;
  u32int value = 0;
  switch (registerOffset)
  {
    case PM_WKEN:
    {
      value = prm->prmWkenWkup;
      break;
    }
    case PM_MPUGRPSEL:
    {
      value = prm->prmMpugrpselWkup;
      break;
    }
    case PM_IVA2GRPSEL:
    {
      value = prm->prmIva2grpselWkup;
      break;
    }
    case PM_WKST:
    {
      value = prm->prmWkstWkup;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_PWSTCTRL:
    case PM_PWSTST:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
    {
      printf("offset %x" EOL, registerOffset);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadWakeUpPrm offset %x value %#.8x" EOL, registerOffset, value);
  return value;
}

void storePrm(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_PRM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %x, val %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  ASSERT(size == WORD, ERROR_BAD_ACCESS_SIZE);

  struct PowerAndResetManager *const prm = context->vm.prMan;
  const u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case Clock_Control_Reg_PRM:
    {
      storeClockControlPrm(prm, phyAddr, value);
      break;
    }
    case IVA2_PRM:
    {
      storeIva2Prm(prm, phyAddr, value);
      break;
    }
    case OCP_System_Reg_PRM:
    {
      storeOcpSystemPrm(prm, phyAddr, value);
      break;
    }
    case MPU_PRM:
    {
      storeMpuPrm(prm, phyAddr, value);
      break;
    }
    case CORE_PRM:
    {
      storeCorePrm(prm, phyAddr, value);
      break;
    }
    case SGX_PRM:
    {
      storeSgxPrm(prm, phyAddr, value);
      break;
    }
    case WKUP_PRM:
    {
      storeWakeUpPrm(prm, phyAddr, value);
      break;
    }
    case DSS_PRM:
    {
      storeDssPrm(prm, phyAddr, value);
      break;
    }
    case CAM_PRM:
    {
      storeCamPrm(prm, phyAddr, value);
      break;
    }
    case PER_PRM:
    {
      storePerPrm(prm, phyAddr, value);
      break;
    }
    case EMU_PRM:
    {
      storeEmuPrm(prm, phyAddr, value);
      break;
    }
    case NEON_PRM:
    {
      storeNeonPrm(prm, phyAddr, value);
      break;
    }
    case USBHOST_PRM:
    {
      storeUsbhostPrm(prm, phyAddr, value);
      break;
    }
    case Global_Reg_PRM:
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeCamPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - CAM_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeCamPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      prm->prmWkdepCam = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlCam = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeClockControlPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - Clock_Control_Reg_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeClockControlPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeCorePrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - CORE_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeCorePrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlCore = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeDssPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - DSS_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeDssPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      prm->prmWkdepDss = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlDss = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeEmuPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - EMU_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeEmuPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }
}

static void storeIva2Prm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - IVA2_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeIva2Prm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      prm->prmWkdepIva2 = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlIva2 = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeMpuPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - MPU_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeMpuPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      prm->prmWkdepMpu = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlMpu = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeNeonPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - NEON_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeNeonPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      prm->prmWkdepNeon = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlNeon = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeOcpSystemPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - OCP_System_Reg_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeOcpSystemPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PRM_REVISION_OCP:
    {
      DIE_NOW(NULL, "storing to R/O register (revision).");
      break;
    }
    case PRM_SYSCONFIG_OCP:
    {
      prm->prmSysConfigOcp = value & PRM_SYSCONFIG_OCP_AUTOIDLE; // all other bits are reserved
      break;
    }
    case PRM_IRQSTATUS_MPU_OCP:
    {
      DIE_NOW(NULL, "store to IRQSTATUS. investigate.");
      break;
    }
    case PRM_IRQENABLE_MPU_OCP:
    {
      DIE_NOW(NULL, "store to IRQENABLE. investigate.");
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends
}

static void storePerPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - PER_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storePerPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlPer = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeSgxPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - SGX_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeSgxPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlSgx = value;
      break;
    }
    case PM_WKDEP:
    {
      prm->prmWkdepSgx = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeUsbhostPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - USBHOST_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeUsbhostPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case PM_WKDEP:
    {
      prm->prmWkdepUsbhost = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlUsbhost = value;
      break;
    }
#ifdef CONFIG_GUEST_ANDROID
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

static void storeWakeUpPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - WKUP_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeWakeUpPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
#ifdef CONFIG_GUEST_ANDROID
    case PM_WKDEP:
    case PM_PWSTCTRL:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
#endif
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}
