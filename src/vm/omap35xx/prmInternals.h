#ifndef __VM__OMAP_35XX__PRM_INTERNALS_H__
#define __VM__OMAP_35XX__PRM_INTERNALS_H__

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
#define CM_SLEEPDEP              0x00000044 /* sleep transition dependency, a register from certain
                                             * CM devices mistakenly used on PRM by the unified
                                             * OMAP2+ drivers in Linux kernels >= v2.6.37-rc1. */
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
 * This unknown register offset is used by some linux kernels.
 * It tries to to read-modify-write this registers
 *   which belong to no module.
 */
#define PM_UNKNOWN               0x00000044 // unknown register

#endif /* __VM__OMAP_35XX__PRM_INTERNALS_H__ */
