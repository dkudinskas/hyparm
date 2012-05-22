#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"

#include "memoryManager/memoryConstants.h" // for BEAGLE_RAM_START/END

#include "vm/omap35xx/sysControlModule.h"


/* instances live at these physical addresses */
#define SYS_CTRL_MOD_INTERFACE          0x48002000
#define SYS_CTRL_MOD_INTERFACE_SIZE           0x24 // 36 bytes
#define SYS_CTRL_MOD_PADCONFS           0x48002030
#define SYS_CTRL_MOD_PADCONFS_SIZE           0x236 // 566 bytes
#define SYS_CTRL_MOD_GENERAL            0x48002270
#define SYS_CTRL_MOD_GENERAL_SIZE            0x2FF // 767 bytes
#define SYS_CTRL_MOD_MEM_WKUP           0x48002600
#define SYS_CTRL_MOD_MEM_WKUP_SIZE           0x400 // 1K byte
#define SYS_CTRL_MOD_PADCONFS_WKUP      0x48002A00
#define SYS_CTRL_MOD_PADCONFS_WKUP_SIZE       0x52 // 82 bytes
#define SYS_CTRL_MOD_GENERAL_WKUP       0x48002A5C
#define SYS_CTRL_MOD_GENERAL_WKUP_SIZE        0x1F // 31 bytes
#define SYS_CTRL_MOD_PADCONFS_ETK       0x480025D8
#define SYS_CTRL_MOD_PADCONFS_ETK_SIZE        0x24 // 36 bytes


/************************
 * REGISTER DEFINITIONS *
 ************************/

// SYS_CTRL_MOD_INTERFACE      0x48002000 base
#define CONTROL_REVISION                               0x00000000
#define CONTROL_SYSCONFIG                              0x00000010
// SYS_CTRL_MOD_PADCONFS       0x48002030 base
#define CONTROL_PADCONF_SDRC_D0                        0x00000000
#define CONTROL_PADCONF_SDRC_D2                        0x00000004
#define CONTROL_PADCONF_SDRC_D4                        0x00000008
#define CONTROL_PADCONF_SDRC_D6                        0x0000000C
#define CONTROL_PADCONF_SDRC_D8                        0x00000010
#define CONTROL_PADCONF_SDRC_D10                       0x00000014
#define CONTROL_PADCONF_SDRC_D12                       0x00000018
#define CONTROL_PADCONF_SDRC_D14                       0x0000001C
#define CONTROL_PADCONF_SDRC_D16                       0x00000020
#define CONTROL_PADCONF_SDRC_D18                       0x00000024
#define CONTROL_PADCONF_SDRC_D20                       0x00000028
#define CONTROL_PADCONF_SDRC_D22                       0x0000002C
#define CONTROL_PADCONF_SDRC_D24                       0x00000030
#define CONTROL_PADCONF_SDRC_D26                       0x00000034
#define CONTROL_PADCONF_SDRC_D28                       0x00000038
#define CONTROL_PADCONF_SDRC_D30                       0x0000003C
#define CONTROL_PADCONF_SDRC_CLK                       0x00000040
#define CONTROL_PADCONF_SDRC_DQS1                      0x00000044
#define CONTROL_PADCONF_SDRC_DQS3                      0x00000048
#define CONTROL_PADCONF_GPMC_A2                        0x0000004C
#define CONTROL_PADCONF_GPMC_A4                        0x00000050
#define CONTROL_PADCONF_GPMC_A6                        0x00000054
#define CONTROL_PADCONF_GPMC_A8                        0x00000058
#define CONTROL_PADCONF_GPMC_A10                       0x0000005C
#define CONTROL_PADCONF_GPMC_D1                        0x00000060
#define CONTROL_PADCONF_GPMC_D3                        0x00000064
#define CONTROL_PADCONF_GPMC_D5                        0x00000068
#define CONTROL_PADCONF_GPMC_D7                        0x0000006C
#define CONTROL_PADCONF_GPMC_D9                        0x00000070
#define CONTROL_PADCONF_GPMC_D11                       0x00000074
#define CONTROL_PADCONF_GPMC_D13                       0x00000078
#define CONTROL_PADCONF_GPMC_D15                       0x0000007C
#define CONTROL_PADCONF_GPMC_NCS1                      0x00000080
#define CONTROL_PADCONF_GPMC_NCS3                      0x00000084
#define CONTROL_PADCONF_GPMC_NCS5                      0x00000088
#define CONTROL_PADCONF_GPMC_NCS7                      0x0000008C
#define CONTROL_PADCONF_GPMC_NADV_ALE                  0x00000090
#define CONTROL_PADCONF_GPMC_NWE                       0x00000094
#define CONTROL_PADCONF_GPMC_NBE1                      0x00000098
#define CONTROL_PADCONF_GPMC_WAIT0                     0x0000009C
#define CONTROL_PADCONF_GPMC_WAIT2                     0x000000A0
#define CONTROL_PADCONF_DSS_PCLK                       0x000000A4
#define CONTROL_PADCONF_DSS_VSYNC                      0x000000A8
#define CONTROL_PADCONF_DSS_DATA0                      0x000000AC
#define CONTROL_PADCONF_DSS_DATA2                      0x000000B0
#define CONTROL_PADCONF_DSS_DATA4                      0x000000B4
#define CONTROL_PADCONF_DSS_DATA6                      0x000000B8
#define CONTROL_PADCONF_DSS_DATA8                      0x000000BC
#define CONTROL_PADCONF_DSS_DATA10                     0x000000C0
#define CONTROL_PADCONF_DSS_DATA12                     0x000000C4
#define CONTROL_PADCONF_DSS_DATA14                     0x000000C8
#define CONTROL_PADCONF_DSS_DATA16                     0x000000CC
#define CONTROL_PADCONF_DSS_DATA18                     0x000000D0
#define CONTROL_PADCONF_DSS_DATA20                     0x000000D4
#define CONTROL_PADCONF_DSS_DATA22                     0x000000D8
#define CONTROL_PADCONF_CAM_HS                         0x000000DC
#define CONTROL_PADCONF_CAM_XCLKA                      0x000000E0
#define CONTROL_PADCONF_CAM_FLD                        0x000000E4
#define CONTROL_PADCONF_CAM_D1                         0x000000E8
#define CONTROL_PADCONF_CAM_D3                         0x000000EC
#define CONTROL_PADCONF_CAM_D5                         0x000000F0
#define CONTROL_PADCONF_CAM_D7                         0x000000F4
#define CONTROL_PADCONF_CAM_D9                         0x000000F8
#define CONTROL_PADCONF_CAM_D11                        0x000000FC
#define CONTROL_PADCONF_CAM_WEN                        0x00000100
#define CONTROL_PADCONF_CSI2_DX0                       0x00000104
#define CONTROL_PADCONF_CSI2_DX1                       0x00000108
#define CONTROL_PADCONF_MCBSP2_FSX                     0x0000010C
#define CONTROL_PADCONF_MCBSP2_DR                      0x00000110
#define CONTROL_PADCONF_MMC1_CLK                       0x00000114
#define CONTROL_PADCONF_MMC1_DAT0                      0x00000118
#define CONTROL_PADCONF_MMC1_DAT2                      0x0000011C
#define CONTROL_PADCONF_MMC1_DAT4                      0x00000120
#define CONTROL_PADCONF_MMC1_DAT6                      0x00000124
#define CONTROL_PADCONF_MMC2_CLK                       0x00000128
#define CONTROL_PADCONF_MMC2_DAT0                      0x0000012C
#define CONTROL_PADCONF_MMC2_DAT2                      0x00000130
#define CONTROL_PADCONF_MMC2_DAT4                      0x00000134
#define CONTROL_PADCONF_MMC2_DAT6                      0x00000138
#define CONTROL_PADCONF_MCBSP3_DX                      0x0000013C
#define CONTROL_PADCONF_MCBSP3_CLKX                    0x00000140
#define CONTROL_PADCONF_UART2_CTS                      0x00000144
#define CONTROL_PADCONF_UART2_TX                       0x00000148
#define CONTROL_PADCONF_UART1_TX                       0x0000014C
#define CONTROL_PADCONF_UART1_CTS                      0x00000150
#define CONTROL_PADCONF_MCBSP4_CLKX                    0x00000154
#define CONTROL_PADCONF_MCBSP4_DX                      0x00000158
#define CONTROL_PADCONF_MCBSP1_CLKR                    0x0000015C
#define CONTROL_PADCONF_MCBSP1_DX                      0x00000160
#define CONTROL_PADCONF_MCBSP_CLKS                     0x00000164
#define CONTROL_PADCONF_MCBSP1_CLKX                    0x00000168
#define CONTROL_PADCONF_UART3_RTS_SD                   0x0000016C
#define CONTROL_PADCONF_UART3_TX_IRTX                  0x00000170
#define CONTROL_PADCONF_HSUSB0_STP                     0x00000174
#define CONTROL_PADCONF_HSUSB0_NXT                     0x00000178
#define CONTROL_PADCONF_HSUSB0_DATA1                   0x0000017C
#define CONTROL_PADCONF_HSUSB0_DATA3                   0x00000180
#define CONTROL_PADCONF_HSUSB0_DATA5                   0x00000184
#define CONTROL_PADCONF_HSUSB0_DATA7                   0x00000188
#define CONTROL_PADCONF_I2C1_SDA                       0x0000018C
#define CONTROL_PADCONF_I2C2_SDA                       0x00000190
#define CONTROL_PADCONF_I2C3_SDA                       0x00000194
#define CONTROL_PADCONF_MCSPI1_CLK                     0x00000198
#define CONTROL_PADCONF_MCSPI1_SOMI                    0x0000019C
#define CONTROL_PADCONF_MCSPI1_CS1                     0x000001A0
#define CONTROL_PADCONF_MCSPI1_CS3                     0x000001A4
#define CONTROL_PADCONF_MCSPI2_SIMO                    0x000001A8
#define CONTROL_PADCONF_MCSPI2_CS0                     0x000001AC
#define CONTROL_PADCONF_SYS_NIRQ                       0x000001B0
#define CONTROL_PADCONF_SAD2D_MCAD0                    0x000001B4
#define CONTROL_PADCONF_SAD2D_MCAD2                    0x000001B8
#define CONTROL_PADCONF_SAD2D_MCAD4                    0x000001BC
#define CONTROL_PADCONF_SAD2D_MCAD6                    0x000001C0
#define CONTROL_PADCONF_SAD2D_MCAD8                    0x000001C4
#define CONTROL_PADCONF_SAD2D_MCAD10                   0x000001C8
#define CONTROL_PADCONF_SAD2D_MCAD12                   0x000001CC
#define CONTROL_PADCONF_SAD2D_MCAD14                   0x000001D0
#define CONTROL_PADCONF_SAD2D_MCAD16                   0x000001D4
#define CONTROL_PADCONF_SAD2D_MCAD18                   0x000001D8
#define CONTROL_PADCONF_SAD2D_MCAD20                   0x000001DC
#define CONTROL_PADCONF_SAD2D_MCAD22                   0x000001E0
#define CONTROL_PADCONF_SAD2D_MCAD24                   0x000001E4
#define CONTROL_PADCONF_SAD2D_MCAD26                   0x000001E8
#define CONTROL_PADCONF_SAD2D_MCAD28                   0x000001EC
#define CONTROL_PADCONF_SAD2D_MCAD30                   0x000001F0
#define CONTROL_PADCONF_SAD2D_MCAD32                   0x000001F4
#define CONTROL_PADCONF_SAD2D_MCAD34                   0x000001F8
#define CONTROL_PADCONF_SAD2D_MCAD36                   0x000001FC
#define CONTROL_PADCONF_SAD2D_NRESPWRON                0x00000200
#define CONTROL_PADCONF_SAD2D_ARMNIRQ                  0x00000204
#define CONTROL_PADCONF_SAD2D_SPINT                    0x00000208
#define CONTROL_PADCONF_SAD2D_DMAREQ0                  0x0000020C
#define CONTROL_PADCONF_SAD2D_DMAREQ2                  0x00000210
#define CONTROL_PADCONF_SAD2D_NTRST                    0x00000214
#define CONTROL_PADCONF_SAD2D_TDO                      0x00000218
#define CONTROL_PADCONF_SAD2D_TCK                      0x0000021C
#define CONTROL_PADCONF_SAD2D_MSTDBY                   0x00000220
#define CONTROL_PADCONF_SAD2D_IDLEACK                  0x00000224
#define CONTROL_PADCONF_SAD2D_SWRITE                   0x00000228
#define CONTROL_PADCONF_SAD2D_SREAD                    0x0000022C
#define CONTROL_PADCONF_SAD2D_SBUSFLAG                 0x00000230
#define CONTROL_PADCONF_SDRC_CKE1                      0x00000234
#define CONTROL_PADCONF_ETK_CLK                        0x000005A8
#define CONTROL_PADCONF_ETK_D0                         0x000005AC
#define CONTROL_PADCONF_ETK_D2                         0x000005B0
#define CONTROL_PADCONF_ETK_D4                         0x000005B4
#define CONTROL_PADCONF_ETK_D6                         0x000005B8
#define CONTROL_PADCONF_ETK_D8                         0x000005BC
#define CONTROL_PADCONF_ETK_D10                        0x000005C0
#define CONTROL_PADCONF_ETK_D12                        0x000005C4
#define CONTROL_PADCONF_ETK_D14                        0x000005C8
// SYS_CTRL_MOD_GENERAL        0x48002270 base
#define CONTROL_PADCONF_OFF                            0x00000000
#define CONTROL_DEVCONF0                               0x00000004
#define CONTROL_MEM_DFTRW0                             0x00000008
#define CONTROL_MEM_DFTRW1                             0x0000000C
#define CONTROL_MSUSPENDMUX_0                          0x00000020
#define CONTROL_MSUSPENDMUX_1                          0x00000024
#define CONTROL_MSUSPENDMUX_2                          0x00000028
#define CONTROL_MSUSPENDMUX_3                          0x0000002C
#define CONTROL_MSUSPENDMUX_4                          0x00000030
#define CONTROL_MSUSPENDMUX_5                          0x00000034
#define CONTROL_SEC_CTRL                               0x00000040
#define CONTROL_DEVCONF1                               0x00000068
#define CONTROL_CSIRXFE                                0x0000006C
#define CONTROL_SEC_STATUS                             0x00000070
#define CONTROL_SEC_ERR_STATUS                         0x00000074
#define CONTROL_SEC_ERR_STATUS_DEBUG                   0x00000078
#define CONTROL_STATUS                                 0x00000080
#define CONTROL_GENERAL_PURPOSE_STATUS                 0x00000084
#define CONTROL_RPUB_KEY_H_0                           0x00000090
#define CONTROL_RPUB_KEY_H_1                           0x00000094
#define CONTROL_RPUB_KEY_H_2                           0x00000098
#define CONTROL_RPUB_KEY_H_3                           0x0000009C
#define CONTROL_RPUB_KEY_H_4                           0x000000A0
/* not accessible on the beagle?
#define CONTROL_RAND_KEY_0                             0x000000A8
#define CONTROL_RAND_KEY_1                             0x000000AC
#define CONTROL_RAND_KEY_2                             0x000000B0
#define CONTROL_RAND_KEY_3                             0x000000B4
#define CONTROL_CUST_KEY_0                             0x000000B8
#define CONTROL_CUST_KEY_1                             0x000000BC
#define CONTROL_CUST_KEY_2                             0x000000C0
#define CONTROL_CUST_KEY_3                             0x000000C4
*/
#define CONTROL_USB_CONF_0                             0x00000100
#define CONTROL_USB_CONF_1                             0x00000104
#define CONTROL_FUSE_OPP1_VDD1                         0x00000110
#define CONTROL_FUSE_OPP2_VDD1                         0x00000114
#define CONTROL_FUSE_OPP3_VDD1                         0x00000118
#define CONTROL_FUSE_OPP4_VDD1                         0x0000011C
#define CONTROL_FUSE_OPP5_VDD1                         0x00000120
#define CONTROL_FUSE_OPP1_VDD2                         0x00000124
#define CONTROL_FUSE_OPP2_VDD2                         0x00000128
#define CONTROL_FUSE_OPP3_VDD2                         0x0000012C
#define CONTROL_FUSE_SR                                0x00000130
#define CONTROL_CEK_0                                  0x00000134
#define CONTROL_CEK_1                                  0x00000138
#define CONTROL_CEK_2                                  0x0000013C
#define CONTROL_CEK_3                                  0x00000140
#define CONTROL_MSV_0                                  0x00000144
#define CONTROL_CEK_BCH_0                              0x00000148
#define CONTROL_CEK_BCH_1                              0x0000014C
#define CONTROL_CEK_BCH_2                              0x00000150
#define CONTROL_CEK_BCH_3                              0x00000154
#define CONTROL_CEK_BCH_4                              0x00000158
#define CONTROL_MSV_BCH_0                              0x0000015C
#define CONTROL_MSV_BCH_1                              0x00000160
#define CONTROL_SWRV_0                                 0x00000164
#define CONTROL_SWRV_1                                 0x00000168
#define CONTROL_SWRV_2                                 0x0000016C
#define CONTROL_SWRV_3                                 0x00000170
#define CONTROL_SWRV_4                                 0x00000174
#define CONTROL_IVA2_BOOTADDR                          0x00000190
#define CONTROL_IVA2_BOOTMOD                           0x00000194
#define CONTROL_DEBOBS_0                               0x000001B0
#define CONTROL_DEBOBS_1                               0x000001B4
#define CONTROL_DEBOBS_2                               0x000001B8
#define CONTROL_DEBOBS_3                               0x000001BC
#define CONTROL_DEBOBS_4                               0x000001C0
#define CONTROL_DEBOBS_5                               0x000001C4
#define CONTROL_DEBOBS_6                               0x000001C8
#define CONTROL_DEBOBS_7                               0x000001CC
#define CONTROL_DEBOBS_8                               0x000001D0
#define CONTROL_PROG_IO0                               0x000001D4
#define CONTROL_PROG_IO1                               0x000001D8
#define CONTROL_WKUP_CTRL                              0x00000A5C
#define CONTROL_DSS_DPLL_SPREADING                     0x000001E0
#define CONTROL_CORE_DPLL_SPREADING                    0x000001E4
#define CONTROL_PER_DPLL_SPREADING                     0x000001E8
#define CONTROL_USBHOST_DPLL_SPREADING                 0x000001EC
#define CONTROL_SECURE_SDRC_SHARING                    0x000001F0
#define CONTROL_SECURE_SDRC_MCFG0                      0x000001F4
#define CONTROL_SECURE_SDRC_MCFG1                      0x000001F8
#define CONTROL_MODEM_FW_CONFIGURATION_LOCK            0x000001FC
#define CONTROL_MODEM_MEMORY_RESOURCES_CONF            0x00000200
#define CONTROL_MODEM_GPMC_DT_FW_REQ_INFO              0x00000204
#define CONTROL_MODEM_GPMC_DT_FW_RD                    0x00000208
#define CONTROL_MODEM_GPMC_DT_FW_WR                    0x0000020C
#define CONTROL_MODEM_GPMC_BOOT_CODE                   0x00000210
#define CONTROL_MODEM_SMS_RG_ATT1                      0x00000214
#define CONTROL_MODEM_SMS_RG_RDPERM1                   0x00000218
#define CONTROL_MODEM_SMS_RG_WRPERM1                   0x0000021C
#define CONTROL_MODEM_D2D_FW_DEBUG_MODE                0x00000220
#define CONTROL_DPF_OCM_RAM_FW_ADDR_MATCH              0x00000228
#define CONTROL_DPF_OCM_RAM_FW_REQINFO                 0x0000022C
#define CONTROL_DPF_OCM_RAM_FW_WR                      0x00000230
#define CONTROL_DPF_REGION4_GPMC_FW_ADDR_MATCH         0x00000234
#define CONTROL_DPF_REGION4_GPMC_FW_REQINFO            0x00000238
#define CONTROL_DPF_REGION4_GPMC_FW_WR                 0x0000023C
#define CONTROL_DPF_REGION1_IVA2_FW_ADDR_MATCH         0x00000240
#define CONTROL_DPF_REGION1_IVA2_FW_REQINFO            0x00000244
#define CONTROL_DPF_REGION1_IVA2_FW_WR                 0x00000248
#define CONTROL_APE_FW_DEFAULT_SECURE_LOCK             0x0000024C
#define CONTROL_OCMROM_SECURE_DEBUG                    0x00000250
#define CONTROL_EXT_SEC_CONTROL                        0x00000264
#define CONTROL_PBIAS_LITE                             0x000002B0
#define CONTROL_CSI                                    0x000002C0
#define CONTROL_DPF_MAD2D_FW_ADDR_MATCH                0x000002C8
#define CONTROL_DPF_MAD2D_FW_REQINFO                   0x000002CC
#define CONTROL_DPF_MAD2D_FW_WR                        0x000002D0
#define CONTROL_IDCODE                                 0x00307F94 // ??????
// SYS_CTRL_MOD_MEM_WKUP       0x48002600 base
#define CONTROL_SAVE_RESTORE_MEM                       0x00000600 // 0x48002600-0x480029FC
// SYS_CTRL_MOD_PADCONFS_WKUP  0x48002A00 base
#define CONTROL_PADCONF_I2C4_SCL                       0x00000000
#define CONTROL_PADCONF_SYS_32K                        0x00000004
#define CONTROL_PADCONF_SYS_NRESWARM                   0x00000008
#define CONTROL_PADCONF_SYS_BOOT1                      0x0000000C
#define CONTROL_PADCONF_SYS_BOOT3                      0x00000010
#define CONTROL_PADCONF_SYS_BOOT5                      0x00000014
#define CONTROL_PADCONF_SYS_OFF_MODE                   0x00000018
#define CONTROL_PADCONF_JTAG_NTRST                     0x0000001C
#define CONTROL_PADCONF_JTAG_TMS_TMSC                  0x00000020
#define CONTROL_PADCONF_JTAG_EMU0                      0x00000024
#define CONTROL_PADCONF_SAD2D_SWAKEUP                  0x0000004C
#define CONTROL_PADCONF_JTAG_TDO                       0x00000050
// SYS_CTRL_MOD_GENERAL_WKUP   0x48002A60 base
#define CONTROL_SEC_TAP                                0x00000000
#define CONTROL_SEC_EMU                                0x00000004
#define CONTROL_WKUP_DEBOBS_0                          0x00000008
#define CONTROL_WKUP_DEBOBS_1                          0x0000000C
#define CONTROL_WKUP_DEBOBS_2                          0x00000010
#define CONTROL_WKUP_DEBOBS_3                          0x00000014
#define CONTROL_WKUP_DEBOBS_4                          0x00000018
#define CONTROL_SEC_DAP                                0x0000001C

#define STATUS_REGISTER                                0x000001DC
#define OMAP3530_CHIPID                                0x00000C00


static u32int loadInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr);
static u32int loadPadconfsScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr);
static u32int loadGeneralScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr);
static u32int loadMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr);
static u32int loadPadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr);
static u32int loadGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr);
static void storeInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value);
static void storePadconfsScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value);
static void storeGeneralScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value);
static void storeMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value);
static void storePadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value);
static void storeGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value);


void initSysControlModule(virtualMachine *vm)
{
  struct SystemControlModule *scm = (struct SystemControlModule *)calloc(1, sizeof(struct SystemControlModule));
  if (scm == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate system control module.");
  }

  DEBUG(VP_OMAP_35XX_SCM, "Initializing system control module at %p" EOL, scm);

  // register default values
  // SYS_CTRL_MOD_INTERFACE      0x48002000 base, 36 bytes length
  // TODO

  // SYS_CTRL_MOD_PADCONFS       0x48002030 base, 564 bytes length
  scm->ctrlPadConfSdrcD0   = 0x01000100;
  scm->ctrlPadConfSdrcD2   = 0x01000100;
  scm->ctrlPadConfSdrcD4   = 0x01000100;
  scm->ctrlPadConfSdrcD6   = 0x01000100;
  scm->ctrlPadConfSdrcD8   = 0x01000100;
  scm->ctrlPadConfSdrcD10  = 0x01000100;
  scm->ctrlPadConfSdrcD12  = 0x01000100;
  scm->ctrlPadConfSdrcD14  = 0x01000100;
  scm->ctrlPadConfSdrcD16  = 0x01000100;
  scm->ctrlPadConfSdrcD18  = 0x01000100;
  scm->ctrlPadConfSdrcD20  = 0x01000100;
  scm->ctrlPadConfSdrcD22  = 0x01000100;
  scm->ctrlPadConfSdrcD24  = 0x01000100;
  scm->ctrlPadConfSdrcD26  = 0x01000100;
  scm->ctrlPadConfSdrcD28  = 0x01000100;
  scm->ctrlPadConfSdrcD30  = 0x01000100;
  scm->ctrlPadConfSdrcClk  = 0x01000100;
  scm->ctrlPadConfSdrcDqs1 = 0x01000100;
  scm->ctrlPadConfSdrcDqs3 = 0x00000100;

  scm->ctrlPadConfGpmcA2    = 0;
  scm->ctrlPadConfGpmcA4    = 0;
  scm->ctrlPadConfGpmcA6    = 0;
  scm->ctrlPadConfGpmcA8    = 0;
  scm->ctrlPadConfGpmcA10   = 0x01000000;
  scm->ctrlPadConfGpmcD1    = 0x01000100;
  scm->ctrlPadConfGpmcD3    = 0x01000100;
  scm->ctrlPadConfGpmcD5    = 0x01000100;
  scm->ctrlPadConfGpmcD7    = 0x01000100;
  scm->ctrlPadConfGpmcD9    = 0x01000100;
  scm->ctrlPadConfGpmcD11   = 0x01000100;
  scm->ctrlPadConfGpmcD13   = 0x01000100;
  scm->ctrlPadConfGpmcD15   = 0x01000100;
  scm->ctrlPadConfGpmcNcs1  = 0x00180018;
  scm->ctrlPadConfGpmcNcs3  = 0x00180018;
  scm->ctrlPadConfGpmcNcs5  = 0x01010000;
  scm->ctrlPadConfGpmcNcs7  = 0x00000119;
  scm->ctrlPadConfGpmcAle   = 0x01000100;
  scm->ctrlPadConfGpmcNwe   = 0x00000100;
  scm->ctrlPadConfGpmcNbe1  = 0x01000100;
  scm->ctrlPadConfGpmcWait0 = 0x01180118;
  scm->ctrlPadConfGpmcWait2 = 0x01180118;

  scm->ctrlPadConfDssPclk   = 0;
  scm->ctrlPadConfDssVsync  = 0;
  scm->ctrlPadConfDssData0  = 0;
  scm->ctrlPadConfDssData2  = 0;
  scm->ctrlPadConfDssData4  = 0;
  scm->ctrlPadConfDssData6  = 0;
  scm->ctrlPadConfDssData8  = 0;
  scm->ctrlPadConfDssData10 = 0;
  scm->ctrlPadConfDssData12 = 0;
  scm->ctrlPadConfDssData14 = 0;
  scm->ctrlPadConfDssData16 = 0;
  scm->ctrlPadConfDssData18 = 0;
  scm->ctrlPadConfDssData20 = 0;
  scm->ctrlPadConfDssData22 = 0;

  scm->ctrlPadConfCamHs    = 0x01180118;
  scm->ctrlPadConfCamXclka = 0x01180000;
  scm->ctrlPadConfCamFld   = 0x01000004;
  scm->ctrlPadConfCamD1    = 0x01000100;
  scm->ctrlPadConfCamD3    = 0x01000100;
  scm->ctrlPadConfCamD5    = 0x01000100;
  scm->ctrlPadConfCamD7    = 0x01000100;
  scm->ctrlPadConfCamD9    = 0x01000100;
  scm->ctrlPadConfCamD11   = 0x00000100;
  scm->ctrlPadConfCamWen   = 0x00000104;

  scm->ctrlPadConfCsi2Dx0  = 0x01000100;
  scm->ctrlPadConfCsi2Dx1  = 0x01000100;

  scm->ctrlPadConfMcbsp2Fsx = 0x01000100;
  scm->ctrlPadConfMcbsp2Dr  = 0x00000100;

  scm->ctrlPadConfMmc1Clk   = 0x01180018;
  scm->ctrlPadConfMmc1Dat0  = 0x01180018;
  scm->ctrlPadConfMmc1Dat2  = 0x01180018;
  scm->ctrlPadConfMmc1Dat4  = 0x01180018;
  scm->ctrlPadConfMmc1Dat6  = 0x01180018;

  scm->ctrlPadConfMmc2Clk   = 0x011c011c;
  scm->ctrlPadConfMmc2Dat0  = 0x011c011c;
  scm->ctrlPadConfMmc2Dat2  = 0x011c011c;
  scm->ctrlPadConfMmc2Dat4  = 0x011c011c;
  scm->ctrlPadConfMmc2Dat6  = 0x011c011c;

  scm->ctrlPadConfMcbsp3Dx   = 0x01040104;
  scm->ctrlPadConfMcbsp3Clkx = 0x01040104;

  scm->ctrlPadConfUart2Cts = 0x00000118;
  scm->ctrlPadConfUart2Tx  = 0x01040000;

  scm->ctrlPadConfUart1Tx  = 0x00040000;
  scm->ctrlPadConfUart1Cts = 0x01000004;

  scm->ctrlPadConfMcbsp4Clkx = 0x01010101;
  scm->ctrlPadConfMcbsp4Dx   = 0x01010101;
  scm->ctrlPadConfMcbsp1Clkr = 0x001c0004;
  scm->ctrlPadConfMcbsp1Dx   = 0x00040004;
  scm->ctrlPadConfMcbspClks  = 0x00040110;
  scm->ctrlPadConfMcbsp1Clkx = 0x01080004;

  scm->ctrlPadConfUart3RtsSd  = 0x01000000;
  scm->ctrlPadConfUart3TxIrtx = 0x01000000;

  scm->ctrlPadConfHsusb0Stp   = 0x01000018;
  scm->ctrlPadConfHsusb0Nxt   = 0x01000100;
  scm->ctrlPadConfHsusb0Data1 = 0x01000100;
  scm->ctrlPadConfHsusb0Data3 = 0x01000100;
  scm->ctrlPadConfHsusb0Data5 = 0x01000100;
  scm->ctrlPadConfHsusb0Data7 = 0x01180100;

  scm->ctrlPadConfI2c1Sda = 0x011c0118;
  scm->ctrlPadConfI2c2Sda = 0x0118011c;
  scm->ctrlPadConfI2c3Sda = 0x001c0118;

  scm->ctrlPadConfMcspi1Clk  = 0x011c011c;
  scm->ctrlPadConfMcspi1Somi = 0x0108011c;
  scm->ctrlPadConfMcspi1Cs1  = 0x00040008;
  scm->ctrlPadConfMcspi1Cs3  = 0x01130113;

  scm->ctrlPadConfMcspi2Simo = 0x01130113;
  scm->ctrlPadConfMcspi2Cs0  = 0x01130113;

  scm->ctrlPadConfSysNirq    = 0x011c0118;

  scm->ctrlPadConfSad2dMcad0     = 0x01080108;
  scm->ctrlPadConfSad2dMcad2     = 0x01080108;
  scm->ctrlPadConfSad2dMcad4     = 0x01080108;
  scm->ctrlPadConfSad2dMcad6     = 0x01080108;
  scm->ctrlPadConfSad2dMcad8     = 0x01080108;
  scm->ctrlPadConfSad2dMcad10    = 0x01080108;
  scm->ctrlPadConfSad2dMcad12    = 0x01080108;
  scm->ctrlPadConfSad2dMcad14    = 0x01080108;
  scm->ctrlPadConfSad2dMcad16    = 0x01080108;
  scm->ctrlPadConfSad2dMcad18    = 0x01080108;
  scm->ctrlPadConfSad2dMcad20    = 0x01080108;
  scm->ctrlPadConfSad2dMcad22    = 0x01080108;
  scm->ctrlPadConfSad2dMcad24    = 0x01080108;
  scm->ctrlPadConfSad2dMcad26    = 0x01080108;
  scm->ctrlPadConfSad2dMcad28    = 0x01080108;
  scm->ctrlPadConfSad2dMcad30    = 0x01080108;
  scm->ctrlPadConfSad2dMcad32    = 0x01080108;
  scm->ctrlPadConfSad2dMcad34    = 0x01080108;
  scm->ctrlPadConfSad2dMcad36    = 0x01000108;
  scm->ctrlPadConfSad2dNrespwron = 0x01180100;
  scm->ctrlPadConfSad2dArmnirq   = 0x01000100;
  scm->ctrlPadConfSad2dSpint     = 0x01080108;
  scm->ctrlPadConfSad2dDmareq0   = 0x01000100;
  scm->ctrlPadConfSad2dDmareq2   = 0x01000100;
  scm->ctrlPadConfSad2dNtrst     = 0x01000100;
  scm->ctrlPadConfSad2dTdo       = 0x01000100;
  scm->ctrlPadConfSad2dTck       = 0x01000100;
  scm->ctrlPadConfSad2dMstdby    = 0x01000118;
  scm->ctrlPadConfSad2dIdleack   = 0x01000118;
  scm->ctrlPadConfSad2dSwrite    = 0x01000100;
  scm->ctrlPadConfSad2dSread     = 0x01000100;
  scm->ctrlPadConfSad2dSbusflag  = 0x01180100;

  scm->ctrlPadConfSdrcCke1 = 0x01000118;

  scm->ctrlPadConfEtkClk = 0x0013001b;
  scm->ctrlPadConfEtkD0  = 0x01130113;
  scm->ctrlPadConfEtkD2  = 0x01130113;
  scm->ctrlPadConfEtkD4  = 0x01130113;
  scm->ctrlPadConfEtkD6  = 0x01130113;
  scm->ctrlPadConfEtkD8  = 0x01130113;
  scm->ctrlPadConfEtkD10 = 0x00130013;
  scm->ctrlPadConfEtkD12 = 0x01130113;
  scm->ctrlPadConfEtkD14 = 0x01130113;

  // SYS_CTRL_MOD_GENERAL        0x48002270 base, 767 bytes length
  scm->ctrlPadConfOff = 0x00000000;
  scm->ctrlDevConf0 = 0x05000000;
  scm->ctrlMemDftrw0 = 0x00000000;
  scm->ctrlMemDftrw1 = 0x00000000;
  scm->ctrlMsuspendMux0 = 0x00000000;
  scm->ctrlMsuspendMux1 = 0x00000000;
  scm->ctrlMsuspendMux2 = 0x00248000;
  scm->ctrlMsuspendMux3 = 0x00000000;
  scm->ctrlMsuspendMux4 = 0x00000000;
  scm->ctrlMsuspendMux5 = 0x00000000;
  scm->ctrlSecCtrl = 0x00001881;
  scm->ctrlDevConf1 = 0x00000000;
  scm->ctrlCsiRxfe = 0x00000000;
  scm->ctrlSecStatus = 0x00000000;
  scm->ctrlSecErrStatus = 0x00000000;
  scm->ctrlSecErrStatusDbg = 0x00000000;
  scm->ctrlStatus = 0x0000030f;
  scm->ctrlGpStatus = 0x00000000;
  scm->ctrlRpubKeyH0 = 0x00000000;
  scm->ctrlRpubKeyH1 = 0x00000000;
  scm->ctrlRpubKeyH2 = 0x00000000;
  scm->ctrlRpubKeyH3 = 0x00000000;
  scm->ctrlRpubKeyH4 = 0x00000000;
   // not accessible on the beagle?...
  /*
  sysCtrlModule->ctrlRandKey0 = 0x;
  sysCtrlModule->ctrlRandKey1 = 0x;
  sysCtrlModule->ctrlRandKey2 = 0x;
  sysCtrlModule->ctrlRandKey3 = 0x;
  sysCtrlModule->ctrlCustKey0 = 0x;
  sysCtrlModule->ctrlCustKey1 = 0x;
  sysCtrlModule->ctrlCustKey2 = 0x;
  sysCtrlModule->ctrlCustKey3 = 0x;
  */
   // .. up to here
  scm->ctrlUsbConf0 = 0x00000000;
  scm->ctrlUsbConf1 = 0x00000000;
  scm->ctrlFuseOpp1Vdd1 = 0x0099bc84;
  scm->ctrlFuseOpp2Vdd1 = 0x009a88c1;
  scm->ctrlFuseOpp3Vdd1 = 0x00aab48a;
  scm->ctrlFuseOpp4Vdd1 = 0x00aba2e6;
  scm->ctrlFuseOpp5Vdd1 = 0x00ab90d3;
  scm->ctrlFuseOpp1Vdd2 = 0x0099be86;
  scm->ctrlFuseOpp2Vdd2 = 0x009a89c4;
  scm->ctrlFuseOpp3Vdd2 = 0x00aac695;
  scm->ctrlFuseSr = 0x00000a0f;
  scm->ctrlCek0 = 0x00000000;
  scm->ctrlCek1 = 0x00000000;
  scm->ctrlCek2 = 0x00000000;
  scm->ctrlCek3 = 0x00000000;
  scm->ctrlMsv0 = 0x00000000;
  scm->ctrlCekBch0 = 0x00000000;
  scm->ctrlCekBch1 = 0x00000000;
  scm->ctrlCekBch2 = 0x00000000;
  scm->ctrlCekBch3 = 0x00000000;
  scm->ctrlCekBch4 = 0x00000000;
  scm->ctrlMsvBch0 = 0x00000000;
  scm->ctrlMsvBch1 = 0x00000000;
  scm->ctrlSwrv0 = 0x02000000;
  scm->ctrlSwrv1 = 0x00000000;
  scm->ctrlSwrv2 = 0x00008000;
  scm->ctrlSwrv3 = 0x00080100;
  scm->ctrlSwrv4 = 0x00200000;
  scm->ctrlIva2Bootaddr = 0x00000000;
  scm->ctrlIva2Bootmod = 0x00000000;
  scm->ctrlDebobs0 = 0x00000000;
  scm->ctrlDebobs1 = 0x00000000;
  scm->ctrlDebobs2 = 0x00000000;
  scm->ctrlDebobs3 = 0x00000000;
  scm->ctrlDebobs4 = 0x00000000;
  scm->ctrlDebobs5 = 0x00000000;
  scm->ctrlDebobs6 = 0x00000000;
  scm->ctrlDebobs7 = 0x00000000;
  scm->ctrlDebobs8 = 0x00000000;
  scm->ctrlProgIO0 = 0x00007fc0;
  scm->ctrlProgIO1 = 0x0002aaaa;
  scm->ctrlWkupCtrl = 0x00000000; // ??? @ off 0x00000A5C
  scm->ctrlDssDpllSpreading = 0x00000040;
  scm->ctrlCoreDpllSpreading = 0x00000040;
  scm->ctrlPerDpllSpreading = 0x00000040;
  scm->ctrlUsbhostDpllSpreading = 0x00000040;
  scm->ctrlSecSdrcSharing = 0x00002700;
  scm->ctrlSecSdrcMcfg0 = 0x00300000;
  scm->ctrlSecSdrcMcfg1 = 0x00300000;
  scm->ctrlModemFwConfLock = 0x00000000;
  scm->ctrlModemMemResConf = 0x00000000;
  scm->ctrlModemGpmcDtFwReqInfo = 0x0000ffff;
  scm->ctrlModemGpmcDtFwRd = 0x0000ffff;
  scm->ctrlModemGpmcDtFwWr = 0x0000ffff;
  scm->ctrlModemGpmcBootCode = 0x00000000;
  scm->ctrlModemSmsRgAtt1 = 0xffffffff;
  scm->ctrlModemSmsRgRdPerm1 = 0x0000ffff;
  scm->ctrlModemSmsRgWrPerm1 = 0x0000ffff;
  scm->ctrlModemD2dFwDbgMode = 0x00000000;
  scm->ctrlDpfOcmRamFwAddrMatch = 0x00000000;
  scm->ctrlDpfOcmRamFwReqinfo = 0x00000000;
  scm->ctrlDpfOcmRamFwWr = 0x00000000;
  scm->ctrlDpfReg4GpmcFwAddrMatch = 0x00000000;
  scm->ctrlDpfReg4GpmcFwReqinfo = 0x00000000;
  scm->ctrlDpfReg4GpmcFwWr = 0x00000000;
  scm->ctrlDpfReg1Iva2FwAddrMatch = 0x00000000;
  scm->ctrlDpfReg1Iva2FwReqinfo = 0x00000000;
  scm->ctrlDpfReg1Iva2FwWr = 0x00000000;
  scm->ctrlApeFwDefSecLock = 0x00000000;
  scm->ctrlOcmRomSecDbg = 0x00000000;
  scm->ctrlExtSecCtrl = 0x00000002;
  scm->ctrlPbiasLite = 0x00000b87;
  scm->ctrlCsi = 0x03200000;
  scm->ctrlDpfMad2dFwAddrMatch = 0x00000000;
  scm->ctrlDpfMad2dFwReqinfo = 0x00000000;
  scm->ctrlDpfMad2dFwWr = 0x00000000;
  scm->ctrlIdCode = 0x3b7ae02f; // offs 0x00307F94, phys 0x4830A204 out of range
  // SYS_CTRL_MOD_MEM_WKUP       0x48002600 base, 1024 bytes length
  // this is just a memory blob of 1k

  // SYS_CTRL_MOD_PADCONFS_WKUP  0x48002A00 base, 80 bytes length
  scm->ctrlPadConfI2c4Scl      = 0x01180118;
  scm->ctrlPadConfSys32k       = 0x01000100;
  scm->ctrlPadConfSysNreswarm  = 0x01040118;
  scm->ctrlPadConfSysBoot1     = 0x01040104;
  scm->ctrlPadConfSysBoot3     = 0x01040104;
  scm->ctrlPadConfSysBoot5     = 0x00040104;
  scm->ctrlPadConfSysOffMode   = 0x01000100;
  scm->ctrlPadConfJtagNtrst    = 0x01000100;
  scm->ctrlPadConfJtagTmsTmsc  = 0x01000100;
  scm->ctrlPadConfJtagEmu0     = 0x01000100;
  scm->ctrlPadConfSad2dSwakeup = 0;
  scm->ctrlPadConfJtagTdo      = 0;

  // SYS_CTRL_MOD_GENERAL_WKUP   0x48002A60 base, 31 bytes length
  // TODO
  vm->sysCtrlModule = scm;
}

/* load function */
u32int loadSysCtrlModule(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  DEBUG(VP_OMAP_35XX_SCM, "%s load from pAddr: %#.8x, vAddr %#.8x, aSize %x" EOL, dev->deviceName,
      phyAddr, virtAddr, (u32int)size);

  struct SystemControlModule *scm = context->vm.sysCtrlModule;

#ifndef CONFIG_GUEST_ANDROID
  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "SysControlModule: invalid access size.");
  }
#endif

  u32int alignedAddr = phyAddr & ~0x3;
  u32int val = 0;

  if ((alignedAddr >= SYS_CTRL_MOD_INTERFACE)
      && (alignedAddr < (SYS_CTRL_MOD_INTERFACE + SYS_CTRL_MOD_INTERFACE_SIZE)))
  {
    val = loadInterfaceScm(scm, virtAddr, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS + SYS_CTRL_MOD_PADCONFS_SIZE)))
  {
    val = loadPadconfsScm(scm, virtAddr, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL + SYS_CTRL_MOD_GENERAL_SIZE)))
  {
    val = loadGeneralScm(scm, virtAddr, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_MEM_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_MEM_WKUP + SYS_CTRL_MOD_MEM_WKUP_SIZE)))
  {
    val = loadMemWkupScm(scm, virtAddr, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + SYS_CTRL_MOD_PADCONFS_WKUP_SIZE)))
  {
    val = loadPadconfsWkupScm(scm, virtAddr, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL_WKUP + SYS_CTRL_MOD_GENERAL_WKUP_SIZE)))
  {
    val = loadGeneralWkupScm(scm, virtAddr, phyAddr);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_ETK)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_ETK + SYS_CTRL_MOD_PADCONFS_ETK_SIZE)))
  {
    val = loadPadconfsScm(scm, virtAddr, phyAddr);
  }
  else
  {
    DIE_NOW(NULL, "SysControlModule: invalid base module.");
  }

#ifdef CONFIG_GUEST_ANDROID
  /*
   * Registers are 8-, 16-, 32-bit accessible with little endianness.
   */
  val = (val >> ((phyAddr & 0x3) * 8));
  switch (size)
  {
    case BYTE:
    {
      val &= 0xFF;
      break;
    }
    case HALFWORD:
    {
      val &= 0xFFFF;
      break;
    }
    default:
      break;
  }
#endif /* CONFIG_GUEST_ANDROID */
  return val;
}

static u32int loadInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr)
{
  printf("Load from pAddr: %#.8x, vAddr %#.8x" EOL, phyAddr, address);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  return 0;
}


static u32int loadPadconfsScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr)
{

  u32int val = 0;
  u32int reg = phyAddr - SYS_CTRL_MOD_PADCONFS;

  switch (reg & ~0x3)
  {
    case CONTROL_PADCONF_SDRC_D0:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD0;
      break;
    }
    case CONTROL_PADCONF_SDRC_D2:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD2;
      break;
    }
    case CONTROL_PADCONF_SDRC_D4:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD4;
      break;
    }
    case CONTROL_PADCONF_SDRC_D6:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD6;
      break;
    }
    case CONTROL_PADCONF_SDRC_D8:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD8;
      break;
    }
    case CONTROL_PADCONF_SDRC_D10:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD10;
      break;
    }
    case CONTROL_PADCONF_SDRC_D12:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD12;
      break;
    }
    case CONTROL_PADCONF_SDRC_D14:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD14;
      break;
    }
    case CONTROL_PADCONF_SDRC_D16:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD16;
      break;
    }
    case CONTROL_PADCONF_SDRC_D18:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD18;
      break;
    }
    case CONTROL_PADCONF_SDRC_D20:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD20;
      break;
    }
    case CONTROL_PADCONF_SDRC_D22:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD22;
      break;
    }
    case CONTROL_PADCONF_SDRC_D24:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD24;
      break;
    }
    case CONTROL_PADCONF_SDRC_D26:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD26;
      break;
    }
    case CONTROL_PADCONF_SDRC_D28:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD28;
      break;
    }
    case CONTROL_PADCONF_SDRC_D30:
    {
      val = sysCtrlModule->ctrlPadConfSdrcD30;
      break;
    }
    case CONTROL_PADCONF_SDRC_CLK:
    {
      val = sysCtrlModule->ctrlPadConfSdrcClk;
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS1:
    {
      val = sysCtrlModule->ctrlPadConfSdrcDqs1;
      break;
    }
    case CONTROL_PADCONF_SDRC_DQS3:
    {
      val = sysCtrlModule->ctrlPadConfSdrcDqs3;
      break;
    }
    case CONTROL_PADCONF_GPMC_A2:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA2;
      break;
    }
    case CONTROL_PADCONF_GPMC_A4:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA4;
      break;
    }
    case CONTROL_PADCONF_GPMC_A6:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA6;
      break;
    }
    case CONTROL_PADCONF_GPMC_A8:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA8;
      break;
    }
    case CONTROL_PADCONF_GPMC_A10:
    {
      val = sysCtrlModule->ctrlPadConfGpmcA10;
      break;
    }
    case CONTROL_PADCONF_GPMC_D1:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD1;
      break;
    }
    case CONTROL_PADCONF_GPMC_D3:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD3;
      break;
    }
    case CONTROL_PADCONF_GPMC_D5:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD5;
      break;
    }
    case CONTROL_PADCONF_GPMC_D7:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD7;
      break;
    }
    case CONTROL_PADCONF_GPMC_D9:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD9;
      break;
    }
    case CONTROL_PADCONF_GPMC_D11:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD11;
      break;
    }
    case CONTROL_PADCONF_GPMC_D13:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD13;
      break;
    }
    case CONTROL_PADCONF_GPMC_D15:
    {
      val = sysCtrlModule->ctrlPadConfGpmcD15;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS1:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs1;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS3:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs3;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS5:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs5;
      break;
    }
    case CONTROL_PADCONF_GPMC_NCS7:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNcs7;
      break;
    }
    case CONTROL_PADCONF_GPMC_NADV_ALE:
    {
      val = sysCtrlModule->ctrlPadConfGpmcAle;
      break;
    }
    case CONTROL_PADCONF_GPMC_NWE:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNwe;
      break;
    }
    case CONTROL_PADCONF_GPMC_NBE1:
    {
      val = sysCtrlModule->ctrlPadConfGpmcNbe1;
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT0:
    {
      val = sysCtrlModule->ctrlPadConfGpmcWait0;
      break;
    }
    case CONTROL_PADCONF_GPMC_WAIT2:
    {
      val = sysCtrlModule->ctrlPadConfGpmcWait2;
      break;
    }
    case CONTROL_PADCONF_DSS_PCLK:
    {
      val = sysCtrlModule->ctrlPadConfDssPclk;
      break;
    }
    case CONTROL_PADCONF_DSS_VSYNC:
    {
      val = sysCtrlModule->ctrlPadConfDssVsync;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA0:
    {
      val = sysCtrlModule->ctrlPadConfDssData0;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA2:
    {
      val = sysCtrlModule->ctrlPadConfDssData2;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA4:
    {
      val = sysCtrlModule->ctrlPadConfDssData4;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA6:
    {
      val = sysCtrlModule->ctrlPadConfDssData6;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA8:
    {
      val = sysCtrlModule->ctrlPadConfDssData8;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA10:
    {
      val = sysCtrlModule->ctrlPadConfDssData10;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA12:
    {
      val = sysCtrlModule->ctrlPadConfDssData12;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA14:
    {
      val = sysCtrlModule->ctrlPadConfDssData14;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA16:
    {
      val = sysCtrlModule->ctrlPadConfDssData16;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA18:
    {
      val = sysCtrlModule->ctrlPadConfDssData18;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA20:
    {
      val = sysCtrlModule->ctrlPadConfDssData20;
      break;
    }
    case CONTROL_PADCONF_DSS_DATA22:
    {
      val = sysCtrlModule->ctrlPadConfDssData22;
      break;
    }
    case CONTROL_PADCONF_CAM_HS:
    {
      val = sysCtrlModule->ctrlPadConfCamHs;
      break;
    }
    case CONTROL_PADCONF_CAM_XCLKA:
    {
      val = sysCtrlModule->ctrlPadConfCamXclka;
      break;
    }
    case CONTROL_PADCONF_CAM_FLD:
    {
      val = sysCtrlModule->ctrlPadConfCamFld;
      break;
    }
    case CONTROL_PADCONF_CAM_D1:
    {
      val = sysCtrlModule->ctrlPadConfCamD1;
      break;
    }
    case CONTROL_PADCONF_CAM_D3:
    {
      val = sysCtrlModule->ctrlPadConfCamD3;
      break;
    }
    case CONTROL_PADCONF_CAM_D5:
    {
      val = sysCtrlModule->ctrlPadConfCamD5;
      break;
    }
    case CONTROL_PADCONF_CAM_D7:
    {
      val = sysCtrlModule->ctrlPadConfCamD7;
      break;
    }
    case CONTROL_PADCONF_CAM_D9:
    {
      val = sysCtrlModule->ctrlPadConfCamD9;
      break;
    }
    case CONTROL_PADCONF_CAM_D11:
    {
      val = sysCtrlModule->ctrlPadConfCamD11;
      break;
    }
    case CONTROL_PADCONF_CAM_WEN:
    {
      val = sysCtrlModule->ctrlPadConfCamWen;
      break;
    }
    case CONTROL_PADCONF_CSI2_DX0:
    {
      val = sysCtrlModule->ctrlPadConfCsi2Dx0;
      break;
    }
    case CONTROL_PADCONF_CSI2_DX1:
    {
      val = sysCtrlModule->ctrlPadConfCsi2Dx1;
      break;
    }
    case CONTROL_PADCONF_MCBSP2_FSX:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp2Fsx;
      break;
    }
    case CONTROL_PADCONF_MCBSP2_DR:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp2Dr;
      break;
    }
    case CONTROL_PADCONF_MMC1_CLK:
    {
      val = sysCtrlModule->ctrlPadConfMmc1Clk;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT0:
    {
      val = sysCtrlModule->ctrlPadConfMmc1Dat0;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT2:
    {
      val = sysCtrlModule->ctrlPadConfMmc1Dat2;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT4:
    {
      val = sysCtrlModule->ctrlPadConfMmc1Dat4;
      break;
    }
    case CONTROL_PADCONF_MMC1_DAT6:
    {
      val = sysCtrlModule->ctrlPadConfMmc1Dat6;
      break;
    }
    case CONTROL_PADCONF_MMC2_CLK:
    {
      val = sysCtrlModule->ctrlPadConfMmc2Clk;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT0:
    {
      val = sysCtrlModule->ctrlPadConfMmc2Dat0;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT2:
    {
      val = sysCtrlModule->ctrlPadConfMmc2Dat2;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT4:
    {
      val = sysCtrlModule->ctrlPadConfMmc2Dat4;
      break;
    }
    case CONTROL_PADCONF_MMC2_DAT6:
    {
      val = sysCtrlModule->ctrlPadConfMmc2Dat6;
      break;
    }
    case CONTROL_PADCONF_MCBSP3_DX:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp3Dx;
      break;
    }
    case CONTROL_PADCONF_MCBSP3_CLKX:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp3Clkx;
      break;
    }
    case CONTROL_PADCONF_UART2_CTS:
    {
      val = sysCtrlModule->ctrlPadConfUart2Cts;
      break;
    }
    case CONTROL_PADCONF_UART2_TX:
    {
      val = sysCtrlModule->ctrlPadConfUart2Tx;
      break;
    }
    case CONTROL_PADCONF_UART1_TX:
    {
      val = sysCtrlModule->ctrlPadConfUart1Tx;
      break;
    }
    case CONTROL_PADCONF_UART1_CTS:
    {
      val = sysCtrlModule->ctrlPadConfUart1Cts;
      break;
    }
    case CONTROL_PADCONF_MCBSP4_CLKX:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp4Clkx;
      break;
    }
    case CONTROL_PADCONF_MCBSP4_DX:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp4Dx;
      break;
    }
    case CONTROL_PADCONF_MCBSP1_CLKR:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp1Clkr;
      break;
    }
    case CONTROL_PADCONF_MCBSP1_DX:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp1Dx;
      break;
    }
    case CONTROL_PADCONF_MCBSP_CLKS:
    {
      val = sysCtrlModule->ctrlPadConfMcbspClks;
      break;
    }
    case CONTROL_PADCONF_MCBSP1_CLKX:
    {
      val = sysCtrlModule->ctrlPadConfMcbsp1Clkx;
      break;
    }
    case CONTROL_PADCONF_UART3_RTS_SD:
    {
      val = sysCtrlModule->ctrlPadConfUart3RtsSd;
      break;
    }
    case CONTROL_PADCONF_UART3_TX_IRTX:
    {
      val = sysCtrlModule->ctrlPadConfUart3TxIrtx;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_STP:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Stp;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_NXT:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Nxt;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA1:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data1;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA3:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data3;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA5:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data5;
      break;
    }
    case CONTROL_PADCONF_HSUSB0_DATA7:
    {
      val = sysCtrlModule->ctrlPadConfHsusb0Data7;
      break;
    }
    case CONTROL_PADCONF_I2C1_SDA:
    {
      val = sysCtrlModule->ctrlPadConfI2c1Sda;
      break;
    }
    case CONTROL_PADCONF_I2C2_SDA:
    {
      val = sysCtrlModule->ctrlPadConfI2c2Sda;
      break;
    }
    case CONTROL_PADCONF_I2C3_SDA:
    {
      val = sysCtrlModule->ctrlPadConfI2c3Sda;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CLK:
    {
      val = sysCtrlModule->ctrlPadConfMcspi1Clk;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_SOMI:
    {
      val = sysCtrlModule->ctrlPadConfMcspi1Somi;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CS1:
    {
      val = sysCtrlModule->ctrlPadConfMcspi1Cs1;
      break;
    }
    case CONTROL_PADCONF_MCSPI1_CS3:
    {
      val = sysCtrlModule->ctrlPadConfMcspi1Cs3;
      break;
    }
    case CONTROL_PADCONF_MCSPI2_SIMO:
    {
      val = sysCtrlModule->ctrlPadConfMcspi2Simo;
      break;
    }
    case CONTROL_PADCONF_MCSPI2_CS0:
    {
      val = sysCtrlModule->ctrlPadConfMcspi2Cs0;
      break;
    }
    case CONTROL_PADCONF_SYS_NIRQ:
    {
      val = sysCtrlModule->ctrlPadConfSysNirq;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD0:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad0;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD2:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad2;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD4:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad4;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD6:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad6;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD8:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad8;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD10:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad10;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD12:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad12;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD14:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad14;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD16:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad16;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD18:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad18;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD20:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad20;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD22:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad22;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD24:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad24;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD26:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad26;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD28:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad28;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD30:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad30;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD32:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad32;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD34:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad34;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MCAD36:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMcad36;
      break;
    }
    case CONTROL_PADCONF_SAD2D_NRESPWRON:
    {
      val = sysCtrlModule->ctrlPadConfSad2dNrespwron;
      break;
    }
    case CONTROL_PADCONF_SAD2D_ARMNIRQ:
    {
      val = sysCtrlModule->ctrlPadConfSad2dArmnirq;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SPINT:
    {
      val = sysCtrlModule->ctrlPadConfSad2dSpint;
      break;
    }
    case CONTROL_PADCONF_SAD2D_DMAREQ0:
    {
      val = sysCtrlModule->ctrlPadConfSad2dDmareq0;
      break;
    }
    case CONTROL_PADCONF_SAD2D_DMAREQ2:
    {
      val = sysCtrlModule->ctrlPadConfSad2dDmareq2;
      break;
    }
    case CONTROL_PADCONF_SAD2D_NTRST:
    {
      val = sysCtrlModule->ctrlPadConfSad2dNtrst;
      break;
    }
    case CONTROL_PADCONF_SAD2D_TDO:
    {
      val = sysCtrlModule->ctrlPadConfSad2dTdo;
      break;
    }
    case CONTROL_PADCONF_SAD2D_TCK:
    {
      val = sysCtrlModule->ctrlPadConfSad2dTck;
      break;
    }
    case CONTROL_PADCONF_SAD2D_MSTDBY:
    {
      val = sysCtrlModule->ctrlPadConfSad2dMstdby;
      break;
    }
    case CONTROL_PADCONF_SAD2D_IDLEACK:
    {
      val = sysCtrlModule->ctrlPadConfSad2dIdleack;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SWRITE:
    {
      val = sysCtrlModule->ctrlPadConfSad2dSwrite;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SREAD:
    {
      val = sysCtrlModule->ctrlPadConfSad2dSread;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SBUSFLAG:
    {
      val = sysCtrlModule->ctrlPadConfSad2dSbusflag;
      break;
    }
    case CONTROL_PADCONF_SDRC_CKE1:
    {
      val = sysCtrlModule->ctrlPadConfSdrcCke1;
      break;
    }
    case CONTROL_PADCONF_ETK_CLK:
    {
      val = sysCtrlModule->ctrlPadConfEtkClk;
      break;
    }
    case CONTROL_PADCONF_ETK_D0:
    {
      val = sysCtrlModule->ctrlPadConfEtkD0;
      break;
    }
    case CONTROL_PADCONF_ETK_D2:
    {
      val = sysCtrlModule->ctrlPadConfEtkD2;
      break;
    }
    case CONTROL_PADCONF_ETK_D4:
    {
      val = sysCtrlModule->ctrlPadConfEtkD4;
      break;
    }
    case CONTROL_PADCONF_ETK_D6:
    {
      val = sysCtrlModule->ctrlPadConfEtkD6;
      break;
    }
    case CONTROL_PADCONF_ETK_D8:
    {
      val = sysCtrlModule->ctrlPadConfEtkD8;
      break;
    }
    case CONTROL_PADCONF_ETK_D10:
    {
      val = sysCtrlModule->ctrlPadConfEtkD10;
      break;
    }
    case CONTROL_PADCONF_ETK_D12:
    {
      val = sysCtrlModule->ctrlPadConfEtkD12;
      break;
    }
    case CONTROL_PADCONF_ETK_D14:
    {
      val = sysCtrlModule->ctrlPadConfEtkD14;
      break;
    }
    default:
    {
      printf("%s: unimplemented reg addr %#.8x" EOL, __func__, phyAddr);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
    }
  }

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, reg, val);
  return val;
}


static u32int loadGeneralScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - SYS_CTRL_MOD_GENERAL;
  switch (reg)
  {
    case CONTROL_DEVCONF0:
      val = sysCtrlModule->ctrlDevConf0;
      break;
    case CONTROL_DEVCONF1:
      val = sysCtrlModule->ctrlDevConf1;
      break;
    case CONTROL_STATUS:
      val = sysCtrlModule->ctrlStatus;
      break;
    case STATUS_REGISTER:
      val = OMAP3530_CHIPID;
      break;
    default:
      printf("loadGeneralScm: unimplemented reg addr %#.8x" EOL, phyAddr);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  } // switch ends
  DEBUG(VP_OMAP_35XX_SCM, "loadGeneralScm reg %x value %#.8x" EOL, reg, val);
  return val;
}


static u32int loadMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - SYS_CTRL_MOD_MEM_WKUP;

  switch (reg & ~0x3)
  {
    default:
    {
      printf("%s: >>-----> unimplemented reg addr %#.8x" EOL, __func__, phyAddr);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
    }
  }

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, reg, val);
  return val;
}


static u32int loadPadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr)
{
  u32int val = 0;
  u32int reg = phyAddr - SYS_CTRL_MOD_PADCONFS_WKUP;

  switch (reg & ~0x3)
  {
    case CONTROL_PADCONF_I2C4_SCL:
    {
      val = sysCtrlModule->ctrlPadConfI2c4Scl;
      break;
    }
    case CONTROL_PADCONF_SYS_32K:
    {
      val = sysCtrlModule->ctrlPadConfSys32k;
      break;
    }
    case CONTROL_PADCONF_SYS_NRESWARM:
    {
      val = sysCtrlModule->ctrlPadConfSysNreswarm;
      break;
    }
    case CONTROL_PADCONF_SYS_BOOT1:
    {
      val = sysCtrlModule->ctrlPadConfSysBoot1;
      break;
    }
    case CONTROL_PADCONF_SYS_BOOT3:
    {
      val = sysCtrlModule->ctrlPadConfSysBoot3;
      break;
    }
    case CONTROL_PADCONF_SYS_BOOT5:
    {
      val = sysCtrlModule->ctrlPadConfSysBoot5;
      break;
    }
    case CONTROL_PADCONF_SYS_OFF_MODE:
    {
      val = sysCtrlModule->ctrlPadConfSysOffMode;
      break;
    }
    case CONTROL_PADCONF_JTAG_NTRST:
    {
      val = sysCtrlModule->ctrlPadConfJtagNtrst;
      break;
    }
    case CONTROL_PADCONF_JTAG_TMS_TMSC:
    {
      val = sysCtrlModule->ctrlPadConfJtagTmsTmsc;
      break;
    }
    case CONTROL_PADCONF_JTAG_EMU0:
    {
      val = sysCtrlModule->ctrlPadConfJtagEmu0;
      break;
    }
    case CONTROL_PADCONF_SAD2D_SWAKEUP:
    {
      val = sysCtrlModule->ctrlPadConfSad2dSwakeup;
      break;
    }
    case CONTROL_PADCONF_JTAG_TDO:
    {
      val = sysCtrlModule->ctrlPadConfJtagTdo;
      break;
    }
    default:
    {
      printf("%s: unimplemented reg addr %#.8x" EOL, __func__, phyAddr);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      break;
    }
  }

  DEBUG(VP_OMAP_35XX_SCM, "%s reg %x value %#.8x" EOL, __func__, reg, val);
  return val;
}


static u32int loadGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr)
{
  printf("Load from pAddr: %#.8x, vAddr %#.8x" EOL, phyAddr, address);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
  return 0;
}

/* top store function */
void storeSysCtrlModule(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  DEBUG(VP_OMAP_35XX_SCM, "%s store to pAddr: %#.8x, vAddr %#.8x, aSize %x, value %#.8x" EOL,
      dev->deviceName, phyAddr, virtAddr, (u32int)size, value);

  struct SystemControlModule *scm = context->vm.sysCtrlModule;

#ifndef CONFIG_GUEST_ANDROID
  if (size != WORD)
  {
    // only word access allowed in these modules
    DIE_NOW(NULL, "SysControlModule: invalid access size.");
  }
#endif

  u32int alignedAddr = phyAddr & ~0x3;

  if ((alignedAddr >= SYS_CTRL_MOD_INTERFACE)
      && (alignedAddr < (SYS_CTRL_MOD_INTERFACE + SYS_CTRL_MOD_INTERFACE_SIZE)))
  {
    storeInterfaceScm(scm, virtAddr, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS + SYS_CTRL_MOD_PADCONFS_SIZE)))
  {
    storePadconfsScm(scm, virtAddr, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL + SYS_CTRL_MOD_GENERAL_SIZE)))
  {
    storeGeneralScm(scm, virtAddr, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_MEM_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_MEM_WKUP + SYS_CTRL_MOD_MEM_WKUP_SIZE)))
  {
    storeMemWkupScm(scm, virtAddr, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_WKUP + SYS_CTRL_MOD_PADCONFS_WKUP_SIZE)))
  {
    storePadconfsWkupScm(scm, virtAddr, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_GENERAL_WKUP)
        && (alignedAddr < (SYS_CTRL_MOD_GENERAL_WKUP + SYS_CTRL_MOD_GENERAL_WKUP_SIZE)))
  {
    storeGeneralWkupScm(scm, virtAddr, phyAddr, value);
  }
  else if ((alignedAddr >= SYS_CTRL_MOD_PADCONFS_ETK)
        && (alignedAddr < (SYS_CTRL_MOD_PADCONFS_ETK + SYS_CTRL_MOD_PADCONFS_ETK_SIZE)))
  {
    storePadconfsScm(scm, virtAddr, phyAddr, value);
  }
  else
  {
    DIE_NOW(NULL, "Invalid base module.");
  }
}

static void storeInterfaceScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, address, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

static void storePadconfsScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, address, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

static void storeGeneralScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, address, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

static void storeMemWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, address, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

static void storePadconfsWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, address, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}

static void storeGeneralWkupScm(struct SystemControlModule *sysCtrlModule, u32int address, u32int phyAddr, u32int value)
{
  printf("Store to address %#.8x, value %#.8x" EOL, address, value);
  DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
}
