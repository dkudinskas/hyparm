#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/prm.h"
#include "vm/omap35xx/prmInternals.h"


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

static void storeCamPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeClockControlPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeCorePrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeDssPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeEmuPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
static void storeGlobalRegPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value);
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
  prm->prmRstctrlIva2  = 0x7;
  prm->prmRststIva2    = 0x1;
  prm->prmWkdepIva2    = 0xB3;
  prm->prmPwstctrlIva2 = 0xFF0F07;
  prm->prmPwststIva2   = 0xFF7;
  // OCP_system_reg REGISTERS
  prm->prmRevisionOcp     = 0x10;
  prm->prmSysConfigOcp    = 0x1;
  prm->prmIrqStatusMpuOcp = 0x0;
  prm->prmIrqEnableMpuOcp = 0x0;
  // MPU registers
  prm->prmRststMpu    = 0x1;
  prm->prmWkdepMpu    = 0xA5;
  prm->prmPwstctrlMpu = 0x30107;
  prm->prmPwststMpu   = 0xC7;
  // CORE registers
  prm->prmRststCore       = 0x1;
  prm->prmIva2grpselCore  = 0xC33FFE10;
  prm->prmPwstctrlCore    = 0xF0307;
  prm->prmPwststCore      = 0xF7;
  prm->prmIva2grpsel3Core = 0x4;
  // SGX registers
  prm->prmRststSgx    = 0x1;
  prm->prmWkdepSgx    = 0x16;
  prm->prmPwstctrlSgx = 0x30107;
  prm->prmPwststSgx   = 0x3;
  // Wakeup registers
  prm->prmWkenWkup       = 0x3CB;
  prm->prmMpugrpselWkup  = 0x3CB;
  prm->prmIva2grpselWkup = 0;
  prm->prmWkstWkup       = 0;
  // DSS registers
  prm->prmRststDss    = 0x1;
  prm->prmWkenDss     = 0x1;
  prm->prmWkdepDss    = 0x16;
  prm->prmPwstctrlDss = 0x30107;
  prm->prmPwststDss   = 0x3;
  // CAM registers
  prm->prmRststCam    = 0x1;
  prm->prmWkdepCam    = 0x16;
  prm->prmPwstctrlCam = 0x30107;
  prm->prmPwststCam   = 0x3;
  // PER registers
  prm->prmRststPer      = 0x1;
  prm->prmWkenPer       = 0x3EFFF;
  prm->prmMpugrpselPer  = 0x3EFFF;
  prm->prmIva2grpselPer = 0x3EFFF;
  prm->prmWkdepPer      = 0x27;
  prm->prmPwstctrlPer   = 0x30107;
  prm->prmPwststPer     = 0x7;
  // EMU registers
  prm->prmRststEmu  = 0x1;
  prm->prmPwststEmu = 0xC3;
  // NEON registers
  prm->prmRststNeon    = 0x1;
  prm->prmWkdepNeon    = 0x2;
  prm->prmPwstctrlNeon = 0x7;
  prm->prmPwststNeon   = 0x3;
  // USBHOST registers
  prm->prmRststUsbhost    = 0x1;
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
#ifdef CONFIG_MMC_GUEST_ACCESS
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
      printf("pa %08x (offset %x)" EOL, physicalAddress, registerOffset);
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
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring load from invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
#ifdef CONFIG_MMC_GUEST_ACCESS
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
    case Global_Reg_PRM:
    {
      storeGlobalRegPrm(prm, phyAddr, value);
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
    case RM_RSTST:
    {
      if (prm->prmRststCam != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststCam" EOL, __func__);
      }
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case RM_RSTST:
    {
      if (prm->prmRststCore != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststCore" EOL, __func__);
      }
      break;
    }
    case PM_IVA2GRPSEL:
    {
      if (prm->prmIva2grpselCore != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmIva2grpselCore" EOL, __func__);
      }
      break;
    }
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlCore = value;
      break;
    }
    case PM_IVA2GRPSEL3_CORE:
    {
      if (prm->prmIva2grpsel3Core != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmIva2grpsel3Core" EOL, __func__);
      }
      break;
    }
    case PM_WKEN:
    case PM_MPUGRPSEL:
    case PM_WKST:
    case PM_WKST3_CORE:
    case PM_PWSTST:
    case PM_PREPWSTST:
    case PM_WKEN3_CORE:
    case PM_MPUGRPSEL3_CORE:
    {
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    case RM_RSTCTRL:
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}

static void storeDssPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - DSS_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeDssPrm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case RM_RSTST:
    {
      if (prm->prmRststDss != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststDss" EOL, __func__);
      }
      break;
    }
    case PM_WKEN:
    {
      if (prm->prmWkenDss != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmWkdepDss" EOL, __func__);
      }
      break;
    }
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
    case PM_PWSTST:
    case PM_PREPWSTST:
    {
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case RM_RSTST:
    {
      if (prm->prmRststEmu != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststEmu" EOL, __func__);
      }
      break;
    }
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }
}

static void storeGlobalRegPrm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - Global_Reg_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: offset %x value %#.8x" EOL, __func__, registerOffset, value);
  switch (registerOffset)
  {
    case PRM_CLKSRC_CTRL:
    {
      if (prm->prmClkSrcCtrl != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmClkSrcCtrl" EOL, __func__);
      }
      break;
    }
    case PRM_VC_SMPS_SA:
    case PRM_VC_SMPS_VOL_RA:
    case PRM_VC_SMPS_CMD_RA:
    case PRM_VC_CMD_VAL_0:
    case PRM_VC_CMD_VAL_1:
    case PRM_VC_CH_CONF:
    case PRM_VC_I2C_CFG:
    case PRM_VC_BYPASS_VAL:
    case PRM_RSTCTRL:
    case PRM_RSTTIME:
    case PRM_RSTST:
    case PRM_VOLTCTRL:
    case PRM_SRAM_PCHARGE:
    case PRM_OBSR:
    case PRM_VOLTSETUP1:
    case PRM_VOLTOFFSET:
    case PRM_CLKSETUP:
    case PRM_POLCTRL:
    case PRM_VOLTSETUP2:
    default:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
  }
}

static void storeIva2Prm(struct PowerAndResetManager *prm, u32int physicalAddress, u32int value)
{
  const u32int registerOffset = physicalAddress - IVA2_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "storeIva2Prm: offset %x value %#.8x" EOL, registerOffset, value);
  switch (registerOffset)
  {
    case RM_RSTCTRL:
    {
      if (prm->prmRstctrlIva2 != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRstctrlIva2" EOL, __func__);
      }
      break;
    }
    case RM_RSTST:
    {
      if (prm->prmRststIva2 != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststIva2" EOL, __func__);
      }
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case RM_RSTST:
    {
      if (prm->prmRststMpu != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststMpu" EOL, __func__);
      }
      break;
    }
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
    case PM_EVGENCTRL_MPU:
    case PM_EVGENONTIM_MPU:
    case PM_EVGENOFFTIM_MPU:
    case PM_PWSTST:
    case PM_PREPWSTST:
    {
      printf("offset %x value %#.8x" EOL, registerOffset, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case RM_RSTST:
    {
      if (prm->prmRststNeon != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststNeon" EOL, __func__);
      }
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
      if (prm->prmIrqStatusMpuOcp != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmIrqStatusMpuOcp" EOL, __func__);
      }
      break;
    }
    case PRM_IRQENABLE_MPU_OCP:
    {
      if (prm->prmIrqEnableMpuOcp != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmIrqEnableMpuOcp" EOL, __func__);
      }
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
    case RM_RSTST:
    {
      if (prm->prmRststPer != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststPer" EOL, __func__);
      }
      break;
    }
    case PM_WKEN:
    {
      if (prm->prmWkenPer != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmWkdepDss" EOL, __func__);
      }
      break;
    }
    case PM_MPUGRPSEL:
    {
      if (prm->prmMpugrpselPer != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmMpugrpselPer" EOL, __func__);
      }
      break;
    }
    case PM_IVA2GRPSEL:
    {
      if (prm->prmIva2grpselPer != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmIva2grpselPer" EOL, __func__);
      }
      break;
    }
    case PM_WKDEP:
      if (prm->prmWkdepPer != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmWkdepDss" EOL, __func__);
      }
      break;
    case PM_PWSTCTRL:
    {
      prm->prmPwstctrlPer = value;
      break;
    }
    case PM_WKST:
    case PM_PWSTST:
    case PM_PREPWSTST:
    {
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case RM_RSTST:
    {
      if (prm->prmRststSgx != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststSgx" EOL, __func__);
      }
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case RM_RSTST:
    {
      if (prm->prmRststUsbhost != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmRststUsbhost" EOL, __func__);
      }
      break;
    }
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
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
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
    case PM_IVA2GRPSEL:
    {
      if (prm->prmIva2grpselWkup != value)
      {
        DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to prmIva2grpselWkup" EOL, __func__);
      }
      break;
    }
    case PM_WKEN:
    case PM_MPUGRPSEL:
    case PM_WKST:
    case PM_WKDEP:
    case PM_PWSTCTRL:
    case PM_UNKNOWN:
    {
      DEBUG(VP_OMAP_35XX_PRM, "%s: ignoring store to invalid register %x" EOL, __func__, registerOffset);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}
