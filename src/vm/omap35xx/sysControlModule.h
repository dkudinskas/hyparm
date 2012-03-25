#ifndef __VM__OMAP_35XX__SYS_CONTROL_MODULE_H__
#define __VM__OMAP_35XX__SYS_CONTROL_MODULE_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


/* instances live at these physical addresses */
#define SYS_CTRL_MOD_INTERFACE        0x48002000
#define SYS_CTRL_MOD_INTERFACE_SIZE        0x24 // 36 bytes
#define SYS_CTRL_MOD_PADCONFS         0x48002030
#define SYS_CTRL_MOD_PADCONFS_SIZE        0x234 // 564 bytes
#define SYS_CTRL_MOD_GENERAL          0x48002270
#define SYS_CTRL_MOD_GENERAL_SIZE         0x2ff // 767 bytes
#define SYS_CTRL_MOD_MEM_WKUP         0x48002600
#define SYS_CTRL_MOD_MEM_WKUP_SIZE        0x400 // 1K byte
#define SYS_CTRL_MOD_PADCONFS_WKUP    0x48002A00
#define SYS_CTRL_MOD_PADCONFS_WKUP_SIZE    0x50 // 80 bytes
#define SYS_CTRL_MOD_GENERAL_WKUP     0x48002A60
#define SYS_CTRL_MOD_GENERAL_WKUP_SIZE     0x1F // 31 bytes


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
#define CONTROL_PADCONF_SDRC_CKE1                      0x00000230 // ?????
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
#define CONTROL_PADCONF_MCSPI1_CS1                     0x000001A0
#define CONTROL_PADCONF_MCSPI1_CS3                     0x000001A4
#define CONTROL_PADCONF_MCSPI2_SIMO                    0x000001A8
#define CONTROL_PADCONF_MCSPI2_CS0                     0x000001AC
#define CONTROL_PADCONF_SYS_NIRQ                       0x000001B0
#define CONTROL_PADCONF_ETK_CLK                        0x000005D8
#define CONTROL_PADCONF_ETK_D0                         0x000005DC
#define CONTROL_PADCONF_ETK_D2                         0x000005E0
#define CONTROL_PADCONF_ETK_D4                         0x000005E4
#define CONTROL_PADCONF_ETK_D6                         0x000005E8
#define CONTROL_PADCONF_ETK_D8                         0x000005EC
#define CONTROL_PADCONF_ETK_D10                        0x000005F0
#define CONTROL_PADCONF_ETK_D12                        0x000005F4
#define CONTROL_PADCONF_ETK_D14                        0x000005F8
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



void initSysControlModule(void);

/* top load function */
u32int loadSysCtrlModule(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);

u32int loadInterfaceScm(device * dev, u32int address, u32int phyAddr);
u32int loadPadconfsScm(device * dev, u32int address, u32int phyAddr);
u32int loadGeneralScm(device * dev, u32int address, u32int phyAddr);
u32int loadMemWkupScm(device * dev, u32int address, u32int phyAddr);
u32int loadPadconfsWkupScm(device * dev, u32int address, u32int phyAddr);
u32int loadGeneralWkupScm(device * dev, u32int address, u32int phyAddr);

/* top store function */
void storeSysCtrlModule(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

void storeInterfaceScm(device * dev, u32int address, u32int phyAddr, u32int value);
void storePadconfsScm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeGeneralScm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeMemWkupScm(device * dev, u32int address, u32int phyAddr, u32int value);
void storePadconfsWkupScm(device * dev, u32int address, u32int phyAddr, u32int value);
void storeGeneralWkupScm(device * dev, u32int address, u32int phyAddr, u32int value);


struct SystemControlModule
{
  // registers
  // SYS_CTRL_MOD_INTERFACE      0x48002000 base, 36 bytes length
  // TODO
  // SYS_CTRL_MOD_PADCONFS       0x48002030 base, 564 bytes length
  // TODO
  // SYS_CTRL_MOD_GENERAL        0x48002270 base, 767 bytes length
  u32int ctrlPadConfOff;
  u32int ctrlDevConf0;
  u32int ctrlMemDftrw0;
  u32int ctrlMemDftrw1;
  u32int ctrlMsuspendMux0;
  u32int ctrlMsuspendMux1;
  u32int ctrlMsuspendMux2;
  u32int ctrlMsuspendMux3;
  u32int ctrlMsuspendMux4;
  u32int ctrlMsuspendMux5;
  u32int ctrlSecCtrl;
  u32int ctrlDevConf1;
  u32int ctrlCsiRxfe;
  u32int ctrlSecStatus;
  u32int ctrlSecErrStatus;
  u32int ctrlSecErrStatusDbg;
  u32int ctrlStatus;
  u32int ctrlGpStatus;
  u32int ctrlRpubKeyH0;
  u32int ctrlRpubKeyH1;
  u32int ctrlRpubKeyH2;
  u32int ctrlRpubKeyH3;
  u32int ctrlRpubKeyH4;
  // not accessible on the beagle?...  
  /*
  u32int ctrlRandKey0 = 0x;
  u32int ctrlRandKey1 = 0x;
  u32int ctrlRandKey2 = 0x;
  u32int ctrlRandKey3 = 0x;
  u32int ctrlCustKey0 = 0x;
  u32int ctrlCustKey1 = 0x;
  u32int ctrlCustKey2 = 0x;
  u32int ctrlCustKey3 = 0x;
  */
  // .. up to here 
  u32int ctrlUsbConf0;
  u32int ctrlUsbConf1;
  u32int ctrlFuseOpp1Vdd1;
  u32int ctrlFuseOpp2Vdd1;
  u32int ctrlFuseOpp3Vdd1;
  u32int ctrlFuseOpp4Vdd1;
  u32int ctrlFuseOpp5Vdd1;
  u32int ctrlFuseOpp1Vdd2;
  u32int ctrlFuseOpp2Vdd2;
  u32int ctrlFuseOpp3Vdd2;
  u32int ctrlFuseSr;
  u32int ctrlCek0;
  u32int ctrlCek1;
  u32int ctrlCek2;
  u32int ctrlCek3;
  u32int ctrlMsv0;
  u32int ctrlCekBch0;
  u32int ctrlCekBch1;
  u32int ctrlCekBch2;
  u32int ctrlCekBch3;
  u32int ctrlCekBch4;
  u32int ctrlMsvBch0;
  u32int ctrlMsvBch1;
  u32int ctrlSwrv0;
  u32int ctrlSwrv1;
  u32int ctrlSwrv2;
  u32int ctrlSwrv3;
  u32int ctrlSwrv4;
  u32int ctrlIva2Bootaddr;
  u32int ctrlIva2Bootmod;
  u32int ctrlDebobs0;
  u32int ctrlDebobs1;
  u32int ctrlDebobs2;
  u32int ctrlDebobs3;
  u32int ctrlDebobs4;
  u32int ctrlDebobs5;
  u32int ctrlDebobs6;
  u32int ctrlDebobs7;
  u32int ctrlDebobs8;
  u32int ctrlProgIO0;
  u32int ctrlProgIO1;
  u32int ctrlWkupCtrl; // ??? @ off 0x00000A5C
  u32int ctrlDssDpllSpreading;
  u32int ctrlCoreDpllSpreading;
  u32int ctrlPerDpllSpreading;
  u32int ctrlUsbhostDpllSpreading;
  u32int ctrlSecSdrcSharing;
  u32int ctrlSecSdrcMcfg0;
  u32int ctrlSecSdrcMcfg1;
  u32int ctrlModemFwConfLock;
  u32int ctrlModemMemResConf;
  u32int ctrlModemGpmcDtFwReqInfo;
  u32int ctrlModemGpmcDtFwRd;
  u32int ctrlModemGpmcDtFwWr;
  u32int ctrlModemGpmcBootCode;
  u32int ctrlModemSmsRgAtt1;
  u32int ctrlModemSmsRgRdPerm1;
  u32int ctrlModemSmsRgWrPerm1;
  u32int ctrlModemD2dFwDbgMode;
  u32int ctrlDpfOcmRamFwAddrMatch;
  u32int ctrlDpfOcmRamFwReqinfo;
  u32int ctrlDpfOcmRamFwWr;
  u32int ctrlDpfReg4GpmcFwAddrMatch;
  u32int ctrlDpfReg4GpmcFwReqinfo;
  u32int ctrlDpfReg4GpmcFwWr;
  u32int ctrlDpfReg1Iva2FwAddrMatch;
  u32int ctrlDpfReg1Iva2FwReqinfo;
  u32int ctrlDpfReg1Iva2FwWr;
  u32int ctrlApeFwDefSecLock;
  u32int ctrlOcmRomSecDbg;
  u32int ctrlExtSecCtrl;
  u32int ctrlPbiasLite;
  u32int ctrlCsi;
  u32int ctrlDpfMad2dFwAddrMatch;
  u32int ctrlDpfMad2dFwReqinfo;
  u32int ctrlDpfMad2dFwWr;
  u32int ctrlIdCode; // offs 0x00307F94, phys 0x4830A204 out of range// SYS_CTRL_MOD_MEM_WKUP       0x48002600 base, 1024 bytes length
  // this is just a memory blob of 1k
  // SYS_CTRL_MOD_PADCONFS_WKUP  0x48002A00 base, 80 bytes length
  // TODO
  // SYS_CTRL_MOD_GENERAL_WKUP   0x48002A60 base, 31 bytes length
  // TODO
};


#endif
