#ifndef __VM__OMAP_35XX__INTC_H__
#define __VM__OMAP_35XX__INTC_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


// interrupt number mappings
/*
M_IRQ_0 EMUINT  MPU emulation
M_IRQ_1 COMMTX  MPU emulation
M_IRQ_2 COMMRX  MPU emulation
M_IRQ_3 BENCH  MPU emulation
M_IRQ_4 MCBSP2_ST_IRQ Sidetone MCBSP2 overflow
M_IRQ_5 MCBSP3_ST_IRQ Sidetone MCBSP3 overflow
M_IRQ_6 Reserved      Reserved
M_IRQ_7 sys_nirq      External source (active low)
M_IRQ_8 Reserved      Reserved
M_IRQ_9  SMX_DBG_IRQ     L3 Interconnect error for debug
M_IRQ_10 SMX_APP_IRQ     L3 Interconnect error for application
M_IRQ_11 PRCM_MPU_IRQ    PRCM module IRQ
M_IRQ_12 SDMA_IRQ_0      System DMA request 0
M_IRQ_13 SDMA_IRQ_1      System DMA request 1
M_IRQ_14 SDMA_IRQ_2      System DMA request 2
M_IRQ_15 SDMA_IRQ_3      System DMA request 3
M_IRQ_16 MCBSP1_IRQ      McBSP module 1 IRQ
M_IRQ_17 MCBSP2_IRQ      McBSP module 2 IRQ
M_IRQ_18 SR1_IRQ         SmartReflexTM 1
M_IRQ_19 SR2_IRQ         SmartReflexTM 2
M_IRQ_20 GPMC_IRQ        General-purpose memory controller module
M_IRQ_21 SGX_IRQ         2D/3D graphics module
M_IRQ_22 MCBSP3_IRQ      McBSP module 1 irq
M_IRQ_23 MCBSP4_IRQ      McBSP module 2 irq
M_IRQ_24 CAM_IRQ0        Camera interface request 0
M_IRQ_25 DSS_IRQ         Display subsystem module
M_IRQ_26 MAIL_U0_MPU_IRQ Mailbox user 0 request
M_IRQ_27 MCBSP5_IRQ      McBSP module 5
M_IRQ_28 IVA2_MMU_IRQ    IVA2 MMU
*/
#define GPIO1_IRQ      29
#define GPIO2_IRQ      30
#define GPIO3_IRQ      31
#define GPIO4_IRQ      32
#define GPIO5_IRQ      33
#define GPIO6_IRQ      34
// M_IRQ_35 Reserved
#define WDTIMER3_IRQ   36
#define GPT1_IRQ       37
#define GPT2_IRQ       38
#define GPT3_IRQ       39
#define GPT4_IRQ       40
#define GPT5_IRQ       41
#define GPT6_IRQ       42
#define GPT7_IRQ       43
#define GPT8_IRQ       44
#define GPT9_IRQ       45
#define GPT10_IRQ      46
#define GPT11_IRQ      47
/*
M_IRQ_47 GPT11_IRQ       General-purpose timer module 11
M_IRQ_48 SPI4_IRQ        McSPI module 4
M_IRQ_49 Reserved        Reserved
M_IRQ_50 Reserved        Reserved
M_IRQ_51 Reserved        Reserved
M_IRQ_52 Reserved        Reserved
M_IRQ_53 MG_IRQ
M_IRQ_54 MCBSP4_IRQ_TX
M_IRQ_55 MCBSP4_IRQ_RX
M_IRQ_56 I2C1_IRQ
M_IRQ_57 I2C2_IRQ
M_IRQ_58 HDQ_IRQ        HDQTM/ One-wireTM
M_IRQ_59 McBSP1_IRQ_TX
M_IRQ_60 McBSP1_IRQ_RX
M_IRQ_61 I2C3_IRQ
M_IRQ_62 McBSP2_IRQ_TX
M_IRQ_63 McBSP2_IRQ_RX
M_IRQ_64 Reserved       Reserved
M_IRQ_65 SPI1_IRQ       McSPI module 1
M_IRQ_66 SPI2_IRQ       McSPI module 2
M_IRQ_67 Reserved       Reserved
M_IRQ_68 Reserved       Reserved
M_IRQ_69 Reserved       Reserved
M_IRQ_70 Reserved       Reserved
M_IRQ_71 Reserved       Reserved
*/
#define UART1_IRQ   72
#define UART2_IRQ   73
#define UART3_IRQ   74
/*
M_IRQ_75 PBIAS_IRQ      Merged interrupt for PBIASlite1 and 2
M_IRQ_76 OHCI_IRQ       OHCI controller HSUSB MP Host Interrupt
M_IRQ_77 EHCI_IRQ       EHCI controller HSUSB MP Host Interrupt
M_IRQ_78 TLL_IRQ        HSUSB MP TLL Interrupt
M_IRQ_79 Reserved       Reserved
M_IRQ_80 Reserved       Reserved
M_IRQ_81 MCBSP5_IRQ_TX
M_IRQ_82 MCBSP5_IRQ_RX
M_IRQ_83 MMC1_IRQ       MMC/SD module 1
M_IRQ_84 MS_IRQ         MS-PROTM module
M_IRQ_85 Reserved       Reserved
M_IRQ_86 MMC2_IRQ       MMC/SD module 2
M_IRQ_87 MPU_ICR_IRQ    MPU ICR
M_IRQ_88 RESERVED       Reserved
M_IRQ_89 MCBSP3_IRQ_TX
M_IRQ_90 MCBSP3_IRQ_RX
M_IRQ_91 SPI3_IRQ       McSPI module 3
M_IRQ_92 HSUSB_MC_NINT  High-Speed USB OTG controller
M_IRQ_93 HSUSB_DMA_NINT High-Speed USB OTG DMA controller
M_IRQ_94 MMC3_IRQ       MMC/SD module 3
M_IRQ_95 Reserved       Reserved
*/


struct InterruptController
{
  u32int intcSysConfig;
  u32int intcSysStatus;
  u32int intcSirIrq;
  u32int intcSirFiq;
  u32int intcControl;
  u32int intcProtection;
  u32int intcIdle;
  u32int intcIrqPriority;
  u32int intcFiqPriority;
  u32int intcThreshold;
  u32int intcItr0;
  u32int intcItr1;
  u32int intcItr2;
  u32int intcMir0;
  u32int intcMir1;
  u32int intcMir2;
  u32int intcMirClear0;
  u32int intcMirClear1;
  u32int intcMirClear2;
  u32int intcMirSet0;
  u32int intcMirSet1;
  u32int intcMirSet2;
  u32int intcIsrSet0;
  u32int intcIsrSet1;
  u32int intcIsrSet2;
  u32int intcIsrClear0;
  u32int intcIsrClear1;
  u32int intcIsrClear2;
  u32int intcPendingIrq0;
  u32int intcPendingIrq1;
  u32int intcPendingIrq2;
  u32int intcPendingFiq0;
  u32int intcPendingFiq1;
  u32int intcPendingFiq2;
  u32int intcIlr[96];
};


void clearInterrupt(GCONTXT *context, u32int irqNum);
void initIntc(virtualMachine *vm) __cold__;
void intcDumpRegisters(struct InterruptController *irqController);
bool isFiqPending(struct InterruptController *irqController);
bool isIrqPending(struct InterruptController *irqController);
u32int loadIntc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void setInterrupt(GCONTXT *context, u32int irqNum);
void storeIntc(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

#endif /* __VM__OMAP_35XX__INTC_H__ */
