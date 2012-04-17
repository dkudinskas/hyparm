#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/prm.h"


struct PowerAndResetManager * prMan;

void initPrm(void)
{
  // init function: setup device, reset register values to defaults!
  prMan = (struct PowerAndResetManager *)calloc(1, sizeof(struct PowerAndResetManager));
  if (prMan == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate power and reset manager.");
  }

  DEBUG(VP_OMAP_35XX_PRM, "Initializing Power and reset manager at %p" EOL, prMan);

  // Clock Control registers
  prMan->prmClkSelReg = 0x3;
  prMan->prmClkoutCtrlReg = 0x80;
  // Global reg prm registers
  prMan->prmVcSmpsSa = 0;
  prMan->prmVcSmpsVolRa = 0;
  prMan->prmVcSmpsCmdRa = 0;
  prMan->prmVcCmdVal0 = 0;
  prMan->prmVcCmdVal1 = 0;
  prMan->prmVcChConf = 0;
  prMan->prmVcI2cCfg = 0x18; // ([3]I2C bus high speed mode, [4] repeated start operation mode)
  prMan->prmVcBypassVal = 0;
  prMan->prmRstCtrl = 0;
  prMan->prmRstTime = 0x1006; // [12:8] rstTime2, [0:7] rstTime1
  prMan->prmRstState = 0x1;
  prMan->prmVoltCtrl = 0;
  prMan->prmSramPcharge = 0x50; // sram precharge duration in clk cycles
  prMan->prmClkSrcCtrl = 0x80;
  prMan->prmObsr = 0;
  prMan->prmVoltSetup1 = 0;
  prMan->prmVoltOffset = 0;
  prMan->prmClkSetup = 0;
  prMan->prmPolCtrl = 0xA;
  prMan->prmVoltSetup2 = 0;
  // IVA2 registers
  prMan->prmPwststIva2 = 0xFF7;
  // OCP_system_reg REGISTERS
  prMan->prmRevisionOcp     = 0x10;
  prMan->prmSysConfigOcp    = 0x1;
  prMan->prmIrqStatusMpuOcp = 0x0;
  prMan->prmIrqEnableMpuOcp = 0x0;
  // MPU registers
  prMan->prmPwststMpu = 0xC7;
  // CORE registers
  prMan->prmPwststCore = 0xF7;
  // SGX registers
  prMan->prmPwststSgx = 0x3;
  // Wakeup registers
  prMan->prmWkenWkup       = 0x3CB;
  prMan->prmMpugrpselWkup  = 0x3CB;
  prMan->prmIva2grpselWkup = 0;
  prMan->prmWkstWkup       = 0;
  // DSS registers
  prMan->prmPwststDss = 0x3;
  // CAM registers
  prMan->prmPwststCam = 0x3;
  // PER registers
  prMan->prmPwststPer = 0x7;
  // EMU registers
  prMan->prmPwststEmu = 0xC3;
  // NEON registers
  prMan->prmPwststNeon = 0x3;
  // USBHOST registers
  prMan->prmPwststUsbhost = 0x3;

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
    case PM_PWSTST_CLK:
    {
      printf("Loading invalid register (PM_PWSTST_CLK). Kernel 3.3 bug?" EOL);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadClockControlPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadClockControlPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadGlobalRegPrm(device * dev, u32int address, u32int phyAddr)
{
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
      DIE_NOW(NULL, "loadGlobalRegPrm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_PRM, "loadGlobalRegPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadIva2Prm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - IVA2_PRM;
  switch (reg)
  {
    case PM_PWSTST_IVA2:
    {
      val = prMan->prmPwststIva2;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadIva2Prm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadIva2Prm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadOcpSystemPrm(device * dev, u32int address, u32int phyAddr)
{
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
      DIE_NOW(NULL, "loadOcpSystemPrm loading non existing register!");
  } // switch ends
  DEBUG(VP_OMAP_35XX_PRM, "loadOcpSystemPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadMpuPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - MPU_PRM;
  switch (reg)
  {
    case PM_PWSTST_MPU:
    {
      val = prMan->prmPwststMpu;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadMpuPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadMpuPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadCorePrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - CORE_PRM;
  switch (reg)
  {
    case PM_PWSTST_CORE:
    {
      val = prMan->prmPwststCore;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadCorePrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadCorePrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadSgxPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - SGX_PRM;
  switch (reg)
  {
    case PM_PWSTST_SGX:
    {
      val = prMan->prmPwststSgx;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadSgxPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadSgxPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadWakeUpPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - WKUP_PRM;
  switch (reg)
  {
    case PM_WKEN_WKUP:
    {
      val = prMan->prmWkenWkup;
      break;
    }
    case PM_MPUGRPSEL_WKUP:
    {
      val = prMan->prmMpugrpselWkup;
      break;
    }
    case PM_IVA2GRPSEL_WKUP:
    {
      val = prMan->prmIva2grpselWkup;
      break;
    }
    case PM_WKST_WKUP:
    {
      val = prMan->prmWkstWkup;
      break;
    }
    case PM_PWSTST_WKUP:
    {
      printf("Loading invalid register (PM_PWSTST_WKUP). Kernel 3.3 bug?" EOL);
      val = 0;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadWakeUpPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadWakeUpPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadDssPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - DSS_PRM;
  switch (reg)
  {
    case PM_PWSTST_DSS:
    {
      val = prMan->prmPwststDss;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadDssPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadDssPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadCamPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - CAM_PRM;
  switch (reg)
  {
    case PM_PWSTST_CAM:
    {
      val = prMan->prmPwststCam;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadCamPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadCamPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadPerPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - PER_PRM;
  switch (reg)
  {
    case PM_PWSTST_PER:
    {
      val = prMan->prmPwststPer;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadPerPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadPerPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadEmuPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - EMU_PRM;
  switch (reg)
  {
    case PM_PWSTST_EMU:
    {
      val = prMan->prmPwststEmu;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadEmuPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadEmuPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadNeonPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - NEON_PRM;
  switch (reg)
  {
    case PM_PWSTST_NEON:
    {
      val = prMan->prmPwststNeon;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadNeonPrm loading non existing register!");
    }
  }
  DEBUG(VP_OMAP_35XX_PRM, "loadNeonPrm reg %x value %.8x" EOL, reg, val);
  return val;
}


u32int loadUsbhostPrm(device *dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - USBHOST_PRM;
  switch (reg)
  {
    case PM_PWSTST_USBHOST:
    {
      val = prMan->prmPwststUsbhost;
      break;
    }
    default:
    {
      printf("reg %#.8x addr %#.8x phy %#.8x" EOL, reg, address, phyAddr);
      DIE_NOW(NULL, "loadUsbhostPrm loading non existing register!");
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
    DIE_NOW(NULL, "PRM: invalid access size.");
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
    case OCP_System_Reg_PRM:
    {
      storeOcpSystemPrm(dev, virtAddr, phyAddr, value);
      break;
    }
    case IVA2_PRM:
    case MPU_PRM:
    case CORE_PRM:
    case SGX_PRM:
    case WKUP_PRM:
    case DSS_PRM:
    case CAM_PRM:
    case PER_PRM:
    case EMU_PRM:
    case NEON_PRM:
    case USBHOST_PRM:
    {
      printf("Store to: %s at address %.8x value %.8x" EOL, dev->deviceName, virtAddr, value);
      DIE_NOW(NULL, " unimplemented.");
      break;
    }
    default:
      DIE_NOW(NULL, "PRM: store to invalid base module.");
  }
}

void storeClockControlPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to: %s at address %.8x value %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, " storeClockControlPrm unimplemented.");
}

void storeGlobalRegPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to: %s at address %.8x value %.8x" EOL, dev->deviceName, address, value);
  DIE_NOW(NULL, " storeGlobalRegPrm unimplemented.");
}

void storeOcpSystemPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  u32int reg = phyAddr - OCP_System_Reg_PRM;
  DEBUG(VP_OMAP_35XX_PRM, "%s: storeOcpSystemPrm: store reg %x value %.8x" EOL, dev->deviceName, reg,
      value);
  switch (reg)
  {
    case PRM_REVISION_OCP:
    {
      DIE_NOW(NULL, "storeOcpSystemPrm: storing to R/O register (revision).");
      break;
    }
    case PRM_SYSCONFIG_OCP:
    {
      prMan->prmSysConfigOcp = value & PRM_SYSCONFIG_OCP_AUTOIDLE; // all other bits are reserved
      break;
    }
    case PRM_IRQSTATUS_MPU_OCP:
    {
      DIE_NOW(NULL, "storeOcpSystemPrm store to IRQSTATUS. investigate.");
      break;
    }
    case PRM_IRQENABLE_MPU_OCP:
    {
      DIE_NOW(NULL, "storeOcpSystemPrm store to IRQENABLE. investigate.");
      break;
    }
    default:
      DIE_NOW(NULL, "storeOcpSystemPrm store to non existing register!");
  } // switch ends
}

