#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"

#include "vm/omap35xx/prm.h"
#include "vm/omap35xx/serial.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END
#include "memoryManager/pageTable.h" // for getPhysicalAddress()


extern GCONTXT * getGuestContext(void);

struct PowerAndResetManager * prMan;

void initPrm(void)
{
  // init function: setup device, reset register values to defaults!
  prMan = (struct PowerAndResetManager*)mallocBytes(sizeof(struct PowerAndResetManager));
  if (prMan == 0)
  {
    DIE_NOW(0, "Failed to allocate power and reset manager.");
  }
  else
  {
    memset((void*)prMan, 0x0, sizeof(struct PowerAndResetManager));
#ifdef PRM_DBG
    serial_putstring("Initializing Power and reset manager at 0x");
    serial_putint((u32int)prMan);
    serial_newline();
#endif
  }
  
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
  // OCP_system_reg REGISTERS
  prMan->prmRevisionOcp     = 0x10;
  prMan->prmSysConfigOcp    = 0x1;
  prMan->prmIrqStatusMpuOcp = 0x0;
  prMan->prmIrqEnableMpuOcp = 0x0;
}

/*************************************************************************
 *                           Load  Functions                             *
 *************************************************************************/
u32int loadPrm(device * dev, ACCESS_SIZE size, u32int address)
{
  u32int val = 0;

  //We care about the real physical address of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef PRM_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(" load from physical address: 0x");
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
    DIE_NOW(0, "PRM: invalid access size.");
  }
  
  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case Clock_Control_Reg_PRM:
      val = loadClockControlPrm(dev, address, phyAddr);
      break;
    case Global_Reg_PRM:
      val = loadGlobalRegPrm(dev, address, phyAddr);
      break;
    case OCP_System_Reg_PRM:
      val = loadOcpSystemPrm(dev, address, phyAddr);
      break;
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
      DIE_NOW(0, "PRM load unimplemented.");
      break;
    default:
      DIE_NOW(0, "PRM: invalid base module.");
  }  
  return val;
}


u32int loadClockControlPrm(device * dev, u32int address, u32int phyAddr)
{
  u32int reg = phyAddr - Clock_Control_Reg_PRM;
  if (reg == PRM_CLKSEL)
  {
#ifdef PRM_DBG
    serial_putstring("loadClockControlPrm reg PRM_CLKSEL, val ");
    serial_putint(prMan->prmClkSelReg);
    serial_newline(); 
#endif
    return prMan->prmClkSelReg;
  }
  else if (reg == PRM_CLKOUT_CTRL)
  {
#ifdef PRM_DBG
    serial_putstring("loadClockControlPrm reg PRM_CLKOUT_CTRL, val ");
    serial_putint(prMan->prmClkoutCtrlReg);
    serial_newline(); 
#endif
    return prMan->prmClkoutCtrlReg;
  }
  else
    DIE_NOW(0, "loadClockControlPrm loading from invalid register");
  return 0;
}


u32int loadGlobalRegPrm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - Global_Reg_PRM;
  switch (reg)
  {
    case PRM_VC_SMPS_SA:
      val = prMan->prmVcSmpsSa;
      break;
    case PRM_VC_SMPS_VOL_RA:
      val = prMan->prmVcSmpsVolRa;
      break;
    case PRM_VC_SMPS_CMD_RA:
      val = prMan->prmVcSmpsCmdRa;
      break;
    case PRM_VC_CMD_VAL_0:
      val = prMan->prmVcCmdVal0;
      break;
    case PRM_VC_CMD_VAL_1:
      val = prMan->prmVcCmdVal1;
      break;
    case PRM_VC_CH_CONF:
      val = prMan->prmVcChConf;
      break;
    case PRM_VC_I2C_CFG:
      val = prMan->prmVcI2cCfg;
      break;
    case PRM_VC_BYPASS_VAL:
      val = prMan->prmVcBypassVal;
      break;
    case PRM_RSTCTRL:
      val = prMan->prmRstCtrl;
      break;
    case PRM_RSTTIME:
      val = prMan->prmRstTime;
      break;
    case PRM_RSTST:
      val = prMan->prmRstState;
      break;
    case PRM_VOLTCTRL:
      val = prMan->prmVoltCtrl;
      break;
    case PRM_SRAM_PCHARGE:
      val = prMan->prmSramPcharge;
      break;
    case PRM_CLKSRC_CTRL:
      val = prMan->prmClkSrcCtrl;
      break;
    case PRM_OBSR:
      val = prMan->prmObsr;
      break;
    case PRM_VOLTSETUP1:
      val = prMan->prmVoltSetup1;
      break;
    case PRM_VOLTOFFSET:
      val = prMan->prmVoltOffset;
      break;
    case PRM_CLKSETUP:
      val = prMan->prmClkSetup;
      break;
    case PRM_POLCTRL:
      val = prMan->prmPolCtrl;
      break;
    case PRM_VOLTSETUP2:
      val = prMan->prmVoltSetup2;
      break;
    default:
      DIE_NOW(0, "loadGlobalRegPrm loading non existing register!");
  } // switch ends
#ifdef PRM_DBG
  serial_putstring("loadGlobalRegPrm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
  return val;
}


u32int loadOcpSystemPrm(device * dev, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - OCP_System_Reg_PRM;
  switch (reg)
  {
    case PRM_REVISION_OCP:
      val = prMan->prmRevisionOcp;
      break;
    case PRM_SYSCONFIG_OCP:
      val = prMan->prmSysConfigOcp;
      break;
    case PRM_IRQSTATUS_MPU_OCP:
      val = prMan->prmIrqStatusMpuOcp;
      break;
    case PRM_IRQENABLE_MPU_OCP:
      val = prMan->prmIrqEnableMpuOcp;
      break;
    default:
      DIE_NOW(0, "loadOcpSystemPrm loading non existing register!");
  } // switch ends
#ifdef PRM_DBG
  serial_putstring("loadOcpSystemPrm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(val);
  serial_newline(); 
#endif
  return val;
}


/*************************************************************************
 *                           Store Functions                             *
 *************************************************************************/
void storePrm(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  //We care about the real physical address of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);

#ifdef PRM_DBG
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

  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(0, "PRM: invalid access size.");
  }
  
  u32int base = phyAddr & 0xFFFFFF00;
  switch (base)
  {
    case Clock_Control_Reg_PRM:
      storeClockControlPrm(dev, address, phyAddr, value);
      break;
    case Global_Reg_PRM:
      storeGlobalRegPrm(dev, address, phyAddr, value);
      break;
    case OCP_System_Reg_PRM:
      storeOcpSystemPrm(dev, address, phyAddr, value);
      break;
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
      serial_putstring("Store to: ");
      serial_putstring(dev->deviceName);
      serial_putstring(" at address ");
      serial_putint(address);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      serial_putstring(dev->deviceName);
      DIE_NOW(0, " unimplemented.");
      break;
    default:
      DIE_NOW(0, "PRM: store to invalid base module.");
  }  
}

void storeClockControlPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeClockControlPrm unimplemented.");
}

void storeGlobalRegPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  serial_putstring("Store to: ");
  serial_putstring(dev->deviceName);
  serial_putstring(" at address ");
  serial_putint(address);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline();
  serial_putstring(dev->deviceName);
  DIE_NOW(0, " storeGlobalRegPrm unimplemented.");
}

void storeOcpSystemPrm(device * dev, u32int address, u32int phyAddr, u32int value)
{
  u32int reg = phyAddr - OCP_System_Reg_PRM;
#ifdef PRM_DBG
  serial_putstring("storeOcpSystemPrm reg ");
  serial_putint_nozeros(reg);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline(); 
#endif
  switch (reg)
  {
    case PRM_REVISION_OCP:
      DIE_NOW(0, "storeOcpSystemPrm: storing to R/O register (revision).");
      break;
    case PRM_SYSCONFIG_OCP:
      prMan->prmSysConfigOcp = value & PRM_SYSCONFIG_OCP_AUTOIDLE; // all other bits are reserved
      break;
    case PRM_IRQSTATUS_MPU_OCP:
      DIE_NOW(0, "storeOcpSystemPrm store to IRQSTATUS. investigate.");
      break;
    case PRM_IRQENABLE_MPU_OCP:
      DIE_NOW(0, "storeOcpSystemPrm store to IRQENABLE. investigate.");
      break;
    default:
      DIE_NOW(0, "storeOcpSystemPrm store to non existing register!");
  } // switch ends
}

