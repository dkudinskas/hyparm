#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/prm.h"


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

/*************************************************************************
 *                           Load  Functions                             *
 *************************************************************************/
u32int loadPrm(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  u32int val = 0;

  DEBUG(VP_OMAP_35XX_PRM, "%s load from physical address: %.8x, vAddr %.8x, aSize %x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size);

  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "PRM: invalid access size.");
  }

  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case Clock_Control_Reg_PRM:
    {
      val = loadClockControlPrm(dev, virtAddr, phyAddr);
      break;
    }
    case Global_Reg_PRM:
    {
      val = loadGlobalRegPrm(dev, virtAddr, phyAddr);
      break;
    }
    case IVA2_PRM:
    {
      val = loadIva2Prm(dev, virtAddr, phyAddr);
      break;
    }
    case OCP_System_Reg_PRM:
    {
      val = loadOcpSystemPrm(dev, virtAddr, phyAddr);
      break;
    }
    case MPU_PRM:
    {
      val = loadMpuPrm(dev, virtAddr, phyAddr);
      break;
    }
    case CORE_PRM:
    {
      val = loadCorePrm(dev, virtAddr, phyAddr);
      break;
    }
    case SGX_PRM:
    {
      val = loadSgxPrm(dev, virtAddr, phyAddr);
      break;
    }
    case WKUP_PRM:
    {
      val = loadWakeUpPrm(dev, virtAddr, phyAddr);
      break;
    }
    case DSS_PRM:
    {
      val = loadDssPrm(dev, virtAddr, phyAddr);
      break;
    }
    case CAM_PRM:
    {
      val = loadCamPrm(dev, virtAddr, phyAddr);
      break;
    }
    case PER_PRM:
    {
      val = loadPerPrm(dev, virtAddr, phyAddr);
      break;
    }
    case EMU_PRM:
    {
      val = loadEmuPrm(dev, virtAddr, phyAddr);
      break;
    }
    case NEON_PRM:
    {
      val = loadNeonPrm(dev, virtAddr, phyAddr);
      break;
    }
    case USBHOST_PRM:
    {
      val = loadUsbhostPrm(dev, virtAddr, phyAddr);
      break;
    }
    default:
    {
      printf("PRM: virtual address %#.8x physical address %#.8x" EOL, virtAddr, phyAddr);
      DIE_NOW(NULL, "PRM: invalid base module.");
    }
  }
  return val;
}


u32int loadClockControlPrm(device * dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - Clock_Control_Reg_PRM;
  switch(reg)
  {
    case PRM_CLKSEL:
    {
      val = prMan->prmClkSelReg;
      break;
    }
    case PRM_CLKOUT_CTRL:
    {
      val = prMan->prmClkoutCtrlReg;
      break;
    }
    case PM_WKDEP:
    case PM_PWSTST:
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadClockControlPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadGlobalRegPrm(device * dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - Global_Reg_PRM;

  switch (reg)
  {
    case PRM_VC_SMPS_SA:
    {
      val = prMan->prmVcSmpsSa;
      break;
    }
    case PRM_VC_SMPS_VOL_RA:
    {
      val = prMan->prmVcSmpsVolRa;
      break;
    }
    case PRM_VC_SMPS_CMD_RA:
    {
      val = prMan->prmVcSmpsCmdRa;
      break;
    }
    case PRM_VC_CMD_VAL_0:
    {
      val = prMan->prmVcCmdVal0;
      break;
    }
    case PRM_VC_CMD_VAL_1:
    {
      val = prMan->prmVcCmdVal1;
      break;
    }
    case PRM_VC_CH_CONF:
    {
      val = prMan->prmVcChConf;
      break;
    }
    case PRM_VC_I2C_CFG:
    {
      val = prMan->prmVcI2cCfg;
      break;
    }
    case PRM_VC_BYPASS_VAL:
    {
      val = prMan->prmVcBypassVal;
      break;
    }
    case PRM_RSTCTRL:
    {
      val = prMan->prmRstCtrl;
      break;
    }
    case PRM_RSTTIME:
    {
      val = prMan->prmRstTime;
      break;
    }
    case PRM_RSTST:
    {
      val = prMan->prmRstState;
      break;
    }
    case PRM_VOLTCTRL:
    {
      val = prMan->prmVoltCtrl;
      break;
    }
    case PRM_SRAM_PCHARGE:
    {
      val = prMan->prmSramPcharge;
      break;
    }
    case PRM_CLKSRC_CTRL:
    {
      val = prMan->prmClkSrcCtrl;
      break;
    }
    case PRM_OBSR:
    {
      val = prMan->prmObsr;
      break;
    }
    case PRM_VOLTSETUP1:
    {
      val = prMan->prmVoltSetup1;
      break;
    }
    case PRM_VOLTOFFSET:
    {
      val = prMan->prmVoltOffset;
      break;
    }
    case PRM_CLKSETUP:
    {
      val = prMan->prmClkSetup;
      break;
    }
    case PRM_POLCTRL:
    {
      val = prMan->prmPolCtrl;
      break;
    }
    case PRM_VOLTSETUP2:
    {
      val = prMan->prmVoltSetup2;
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends
  DEBUG(VP_OMAP_35XX_PRM, "loadGlobalRegPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadIva2Prm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - IVA2_PRM;
  switch (reg)
  {
    case PM_WKDEP:
    {
      val = prMan->prmWkdepIva2;
      break;
    }
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlIva2;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststIva2;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    case RM_RSTCTRL:
    case RM_RSTST:
    case PM_PREPWSTST:
    case PRM_IRQSTATUS_IVA2:
    case PRM_IRQENABLE_IVA2:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadIva2Prm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadOcpSystemPrm(device * dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - OCP_System_Reg_PRM;

  switch (reg)
  {
    case PRM_REVISION_OCP:
    {
      val = prMan->prmRevisionOcp;
      break;
    }
    case PRM_SYSCONFIG_OCP:
    {
      val = prMan->prmSysConfigOcp;
      break;
    }
    case PRM_IRQSTATUS_MPU_OCP:
    {
      val = prMan->prmIrqStatusMpuOcp;
      break;
    }
    case PRM_IRQENABLE_MPU_OCP:
    {
      val = prMan->prmIrqEnableMpuOcp;
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends
  DEBUG(VP_OMAP_35XX_PRM, "loadOcpSystemPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadMpuPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - MPU_PRM;
  switch (reg)
  {
    case PM_WKDEP:
    {
      val = prMan->prmWkdepMpu;
      break;
    }
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlMpu;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststMpu;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    case RM_RSTST:
    case PM_EVGENCTRL_MPU:
    case PM_EVGENONTIM_MPU:
    case PM_EVGENOFFTIM_MPU:
    case PM_PREPWSTST:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadMpuPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadCorePrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - CORE_PRM;
  switch (reg)
  {
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlCore;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststCore;
      break;
    }
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadCorePrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadSgxPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - SGX_PRM;
  switch (reg)
  {
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlSgx;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststSgx;
      break;
    }
    case PM_WKDEP:
    {
      val = prMan->prmWkdepSgx;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadSgxPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadWakeUpPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - WKUP_PRM;
  switch (reg)
  {
    case PM_WKEN:
    {
      val = prMan->prmWkenWkup;
      break;
    }
    case PM_MPUGRPSEL:
    {
      val = prMan->prmMpugrpselWkup;
      break;
    }
    case PM_IVA2GRPSEL:
    {
      val = prMan->prmIva2grpselWkup;
      break;
    }
    case PM_WKST:
    {
      val = prMan->prmWkstWkup;
      break;
    }
    case PM_WKDEP:
    case PM_PWSTCTRL:
    case PM_PWSTST:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadWakeUpPrm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadDssPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - DSS_PRM;
  switch (reg)
  {
    case PM_WKDEP:
    {
      val = prMan->prmWkdepDss;
      break;
    }
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlDss;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststDss;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadDssPrm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadCamPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - CAM_PRM;
  switch (reg)
  {
    case PM_WKDEP:
    {
      val = prMan->prmWkdepCam;
      break;
    }
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlCam;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststCam;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadCamPrm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadPerPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - PER_PRM;
  switch (reg)
  {
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlPer;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststPer;
      break;
    }
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadPerPrm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadEmuPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - EMU_PRM;
  switch (reg)
  {
    case PM_PWSTST:
    {
      val = prMan->prmPwststEmu;
      break;
    }
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadEmuPrm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadNeonPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - NEON_PRM;
  switch (reg)
  {
    case PM_WKDEP:
    {
      val = prMan->prmWkdepNeon;
      break;
    }
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlNeon;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststNeon;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadNeonPrm reg %x value %.8x" EOL, reg, val);
  return val;
}

u32int loadUsbhostPrm(device *dev, u32int address, u32int phyAddr)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int val = 0;
  u32int reg = phyAddr - USBHOST_PRM;
  switch (reg)
  {
    case PM_WKDEP:
    {
      val = prMan->prmWkdepUsbhost;
      break;
    }
    case PM_PWSTCTRL:
    {
      val = prMan->prmPwstctrlUsbhost;
      break;
    }
    case PM_PWSTST:
    {
      val = prMan->prmPwststUsbhost;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: loading invalid register." EOL, __func__);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadUsbhostPrm reg %x value %.8x" EOL, reg, val);
  return val;
}

/*************************************************************************
 *                           Store Functions                             *
 *************************************************************************/
void storePrm(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_PRM, "%s store to pAddr: %.8x, vAddr %.8x, aSize %x, val %.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "invalid access size");
  }

  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case Clock_Control_Reg_PRM:
    {
      storeClockControlPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case Global_Reg_PRM:
    {
      storeGlobalRegPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case IVA2_PRM:
    {
      storeIva2Prm(dev, virtAddr, phyAddr, value);
      break;
    }
    case OCP_System_Reg_PRM:
    {
      storeOcpSystemPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case MPU_PRM:
    {
      storeMpuPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case CORE_PRM:
    {
      storeCorePrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case SGX_PRM:
    {
      storeSgxPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case WKUP_PRM:
    {
      storeWakeUpPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case DSS_PRM:
    {
      storeDssPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case CAM_PRM:
    {
      storeCamPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case PER_PRM:
    {
      storePerPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case EMU_PRM:
    {
      storeEmuPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case NEON_PRM:
    {
      storeNeonPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case USBHOST_PRM:
    {
      storeUsbhostPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeClockControlPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  u32int reg = phyAddr - Clock_Control_Reg_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeClockControlPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeGlobalRegPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to: %s at address %.8x value %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

void storeIva2Prm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - IVA2_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeIva2Prm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    {
      prMan->prmWkdepIva2 = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlIva2 = value;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeOcpSystemPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - OCP_System_Reg_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeOcpSystemPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PRM_REVISION_OCP:
    {
      DIE_NOW(NULL, "storing to R/O register (revision).");
      break;
    }
    case PRM_SYSCONFIG_OCP:
    {
      prMan->prmSysConfigOcp = value & PRM_SYSCONFIG_OCP_AUTOIDLE; // all other bits are reserved
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

void storeMpuPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - MPU_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeMpuPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    {
      prMan->prmWkdepMpu = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlMpu = value;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeCorePrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - CORE_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeCorePrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlCore = value;
      break;
    }
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeSgxPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - SGX_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeSgxPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlSgx = value;
      break;
    }
    case PM_WKDEP:
    {
      prMan->prmWkdepSgx = value;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeWakeUpPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  u32int reg = phyAddr - WKUP_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeWakeUpPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    case PM_PWSTCTRL:
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeDssPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - DSS_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeDssPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    {
      prMan->prmWkdepDss = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlDss = value;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeCamPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - CAM_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeCamPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    {
      prMan->prmWkdepCam = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlCam = value;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storePerPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - PER_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storePerPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
        value);
  switch (reg)
  {
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlPer = value;
      break;
    }
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeEmuPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  u32int reg = phyAddr - EMU_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeEmuPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
        value);
  switch (reg)
  {
    case PM_WKDEP:
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  }
}

void storeNeonPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - NEON_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeNeonPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    {
      prMan->prmWkdepNeon = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlNeon = value;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}

void storeUsbhostPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  GCONTXT* context = getGuestContext();
  struct PowerAndResetManager* prMan = context->vm.prMan;

  u32int reg = phyAddr - USBHOST_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeUsbhostPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PM_WKDEP:
    {
      prMan->prmWkdepUsbhost = value;
      break;
    }
    case PM_PWSTCTRL:
    {
      prMan->prmPwstctrlUsbhost = value;
      break;
    }
    case PM_UNKNOWN:
    {
      printf("%s: storing to invalid register." EOL, __func__);
      break;
    }
    default:
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  }
}
