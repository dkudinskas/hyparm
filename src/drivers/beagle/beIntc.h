#ifndef __DRIVERS__BEAGLE__BE_INTC_H__
#define __DRIVERS__BEAGLE__BE_INTC_H__

#include "common/types.h"


#define INTCPS_NR_OF_BANKS          3
#define INTCPS_INTERRUPTS_PER_BANK 32
#define INTCPS_NR_OF_INTERRUPTS    96

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
M_IRQ_72 UART1_IRQ      UART module 1
M_IRQ_73 UART2_IRQ      UART module 2
*/
#define UART1_IRQ      72
#define UART2_IRQ      73
#define UART3_IRQ      74
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


/************************
 * REGISTER DEFINITIONS *
 ************************/

#define REG_INTCPS_REVISION      0x00000000 // REVISION NUMBER

#define REG_INTCPS_SYSCONFIG     0x00000010 // RW config register
#define INTCPS_SYSCONFIG_RESERVED       0xFFFFFFFC // [31:2] reserved
#define INTCPS_SYSCONFIG_SOFTRESET      0x00000002 // [1] write 1 reset INTC 
#define INTCPS_SYSCONFIG_AUTOIDLE       0x00000001 // [0] 0: freerunning 1: autogating

#define REG_INTCPS_SYSSTATUS     0x00000014 // RO status register
#define INTCPS_SYSSTATUS_RESERVED       0xFFFFFFFE // [31:1] reserved
#define INTCPS_SYSSTATUS_SOFTRESET      0x00000001 // [0] 0: resetting 1: reset done 

#define REG_INTCPS_SIR_IRQ       0x00000040 // RO current active irq number 
#define INTCPS_SIR_IRQ_SPURIOUSIRQFLAG  0xFFFFFF80 // [31:7] spurious flag
#define INTCPS_SIR_IRQ_ACTIVEIRQ        0x0000007F // [6:0] active irq number

#define REG_INTCPS_SIR_FIQ       0x00000044 // RO current active fiq number
#define INTCPS_SIR_FIQ_SPURIOUSFIQFLAG  0xFFFFFF80 // [31:7] spurious flag
#define INTCPS_SIR_FIQ_ACTIVEFIQ        0x0000007F // [6:0] active fiq number

#define REG_INTCPS_CONTROL       0x00000048 // RW // control register 
#define INTCPS_CONTROL_RESERVED         0xFFFFFFFC // [31:2] reserved
#define INTCPS_CONTROL_NEWFIQAGR        0x00000002 // [1] write 1: reset fiq and enable new fiq generation
#define INTCPS_CONTROL_NEWIRQAGR        0x00000001 // [0] write 1: reset irq and enable new irq generation

#define REG_INTCPS_PROTECTION    0x0000004C // RW register access protection on/off
#define INTCPS_PROTECTION_RESERVED      0xFFFFFFFE // [31:1] reserved
#define INTCPS_PROTECTION_PROTECTION    0x00000001 // [0] 1: protection on (reg access in priv mode)

#define REG_INTCPS_IDLE          0x00000050 // RW auto-idle / auto-gating
#define INTCPS_IDLE_RESERVED            0xFFFFFFFC // [31:2] reserved
#define INTCPS_IDLE_TURBO               0x00000002 // [1] 0: input synchronizer clock free running 1: ..autogated 
#define INTCPS_IDLE_FUNCIDLE            0x00000001 // [0] 0: functional clock autogated 1: .. freerunning

#define REG_INTCPS_IRQ_PRIORITY  0x00000060 // RW currently active IRQ priority level
#define INTCPS_IRQ_SPURIOUS             0xFFFFFF80 // [31:7] spurious IRQ flag
#define INTCPS_IRQ_PRIORITY             0x0000007F // [6:0] current irq priority

#define REG_INTCPS_FIQ_PRIORITY  0x00000064 // RW currently active FIQ priority level
#define INTCPS_FIQ_PRIORITY_SPURIOUS    0xFFFFFF80 // [31:7] spurious FIQ flag
#define INTCPS_FIQ_PRIORITY_FLAG        0x0000007F // [6:0] current fiq priority

#define REG_INTCPS_THRESHOLD     0x00000068 // RW set priority threshold 
#define INTCPS_THRESHOLD_SPURIOUS       0xFFFFFF00 // [31:8] spurious FIQ flag
#define INTCPS_THRESHOLD_FLAG           0x000000FF // [7:0] current fiq priority

#define REG_INTCPS_ITRn          0x00000080 // RO show raw irq input status before masking 
#define INTCPS_ITRn_ITR                 0xFFFFFFFF // [31:0] interrupt status before masking

#define REG_INTCPS_MIRn          0x00000084 // RW contains the interrupt mask 
#define INTCPS_MIRn_MIR                 0xFFFFFFFF // [31:0] 1: masked 0: unmasked

#define REG_INTCPS_MIR_CLEARn    0x00000088 // WO clear irq mask bits 
#define INTCPS_MIR_CLEARn_MIRCLEAR      0xFFFFFFFF // [31:0] 1: clear mask bit 0: no effect

#define REG_INTCPS_MIR_SETn      0x0000008C // WO set irq mask bits
#define INTCPS_MIR_SETn_MIRSET          0xFFFFFFFF // [31:0] 1: set mir mask bit to 1 0: no effect

#define REG_INTCPS_ISR_SETn      0x00000090 // RW set the software interrupt bits
#define INTCPS_ISR_SETn_ISRSET          0xFFFFFFFF // [31:0] 1: set software interrupt bit to 1 0: no effect

#define REG_INTCPS_ISR_CLEARn    0x00000094 // WO clear software interrupt bits
#define INTCPS_ISR_CLEARn_ISRCLEAR      0xFFFFFFFF // [31:0] 1: clear software interrupt bit to 0 0: no effect

#define REG_INTCPS_PENDING_IRQn  0x00000098 // RO contains the IRQ status after masking
#define INTCPS_PENDING_IRQn_PENDINGIRQ  0xFFFFFFFF // [31:0] irq status after masking

#define REG_INTCPS_PENDING_FIQn  0x0000009C // RO contains the FIQ status after masking
#define INTCPS_PENDING_FIQn_PENDINGFIQ  0xFFFFFFFF // [31:0] fiq status after masking

#define REG_INTCPS_ILRm           0x00000100 // RW contains the priority for the interrupts and the FIQ/IRQ steering

#define INTCPS_FIQNIRQ_FIQ        0x1

void intcBEInit(void);

void unmaskInterruptBE(u32int interruptNumber);

void maskInterruptBE(u32int interruptNumber);

u32int getIrqNumberBE(void);

u32int getFiqNumberBE(void);

void acknowledgeIrqBE(void);

void acknowledgeFiqBE(void);

void intcDumpRegistersBE(void);

void setInterruptMapping(u32int interruptNumber, u32int fiq);

#endif
