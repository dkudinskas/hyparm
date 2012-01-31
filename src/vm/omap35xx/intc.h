#ifndef __VM__OMAP_35XX__INTC_H__
#define __VM__OMAP_35XX__INTC_H__

#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


#define INTCPS_NR_OF_BANKS          3
#define INTCPS_INTERRUPTS_PER_BANK 32
#define INTCPS_NR_OF_INTERRUPTS    96

#define INTC_REVISION                                  0x00000040


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


/************************
 * REGISTER DEFINITIONS *
 ************************/
/* Register address and bit value definitions */
#define MPU_INTC      0x48200000 // Section 10.4.1 p1200

#define REG_INTCPS_REVISION     0x00000000 // REVISION NUMBER


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

#define REG_INTCPS_ITR0          0x00000080 // RO show raw irq input status before masking
#define INTCPS_ITR0_ITR                 0xFFFFFFFF // [31:0] interrupt status before masking
#define REG_INTCPS_ITR1          0x000000a0 // RO show raw irq input status before masking
#define INTCPS_ITR0_ITR                 0xFFFFFFFF // [31:0] interrupt status before masking
#define REG_INTCPS_ITR2          0x000000c0 // RO show raw irq input status before masking
#define INTCPS_ITR0_ITR                 0xFFFFFFFF // [31:0] interrupt status before masking

#define REG_INTCPS_MIR0          0x00000084 // RW contains the interrupt mask
#define INTCPS_MIR0_MIR                 0xFFFFFFFF // [31:0] 1: masked 0: unmasked
#define REG_INTCPS_MIR1          0x000000a4 // RW contains the interrupt mask
#define INTCPS_MIR1_MIR                 0xFFFFFFFF // [31:0] 1: masked 0: unmasked
#define REG_INTCPS_MIR2          0x000000c4 // RW contains the interrupt mask
#define INTCPS_MIR2_MIR                 0xFFFFFFFF // [31:0] 1: masked 0: unmasked

#define REG_INTCPS_MIR_CLEAR0    0x00000088 // WO clear irq mask bits
#define INTCPS_MIR_CLEAR0_MIRCLEAR      0xFFFFFFFF // [31:0] 1: clear mask bit 0: no effect
#define REG_INTCPS_MIR_CLEAR1    0x000000a8 // WO clear irq mask bits
#define INTCPS_MIR_CLEAR1_MIRCLEAR      0xFFFFFFFF // [31:0] 1: clear mask bit 0: no effect
#define REG_INTCPS_MIR_CLEAR2    0x000000c8 // WO clear irq mask bits
#define INTCPS_MIR_CLEAR2_MIRCLEAR      0xFFFFFFFF // [31:0] 1: clear mask bit 0: no effect

#define REG_INTCPS_MIR_SET0      0x0000008C // WO set irq mask bits
#define INTCPS_MIR_SET0_MIRSET          0xFFFFFFFF // [31:0] 1: set mir mask bit to 1 0: no effect
#define REG_INTCPS_MIR_SET1      0x000000AC // WO set irq mask bits
#define INTCPS_MIR_SET1_MIRSET          0xFFFFFFFF // [31:0] 1: set mir mask bit to 1 0: no effect
#define REG_INTCPS_MIR_SET2      0x000000CC // WO set irq mask bits
#define INTCPS_MIR_SET2_MIRSET          0xFFFFFFFF // [31:0] 1: set mir mask bit to 1 0: no effect

#define REG_INTCPS_ISR_SET0      0x00000090 // RW set the software interrupt bits
#define INTCPS_ISR_SET0_ISRSET          0xFFFFFFFF // [31:0] 1: set software interrupt bit to 1 0: no effect
#define REG_INTCPS_ISR_SET1      0x000000B0 // RW set the software interrupt bits
#define INTCPS_ISR_SET1_ISRSET          0xFFFFFFFF // [31:0] 1: set software interrupt bit to 1 0: no effect
#define REG_INTCPS_ISR_SET2      0x000000D0 // RW set the software interrupt bits
#define INTCPS_ISR_SET2_ISRSET          0xFFFFFFFF // [31:0] 1: set software interrupt bit to 1 0: no effect

#define REG_INTCPS_ISR_CLEAR0    0x00000094 // WO clear software interrupt bits
#define INTCPS_ISR_CLEAR0_ISRCLEAR      0xFFFFFFFF // [31:0] 1: clear software interrupt bit to 0 0: no effect
#define REG_INTCPS_ISR_CLEAR1    0x000000B4 // WO clear software interrupt bits
#define INTCPS_ISR_CLEAR1_ISRCLEAR      0xFFFFFFFF // [31:0] 1: clear software interrupt bit to 0 0: no effect
#define REG_INTCPS_ISR_CLEAR2    0x000000D4 // WO clear software interrupt bits
#define INTCPS_ISR_CLEAR2_ISRCLEAR      0xFFFFFFFF // [31:0] 1: clear software interrupt bit to 0 0: no effect

#define REG_INTCPS_PENDING_IRQ0  0x00000098 // RO contains the IRQ status after masking
#define INTCPS_PENDING_IRQ0_PENDINGIRQ  0xFFFFFFFF // [31:0] irq status after masking
#define REG_INTCPS_PENDING_IRQ1  0x000000B8 // RO contains the IRQ status after masking
#define INTCPS_PENDING_IRQ1_PENDINGIRQ  0xFFFFFFFF // [31:0] irq status after masking
#define REG_INTCPS_PENDING_IRQ2  0x000000D8 // RO contains the IRQ status after masking
#define INTCPS_PENDING_IRQ2_PENDINGIRQ  0xFFFFFFFF // [31:0] irq status after masking

#define REG_INTCPS_PENDING_FIQ0  0x0000009C // RO contains the FIQ status after masking
#define INTCPS_PENDING_FIQ0_PENDINGFIQ  0xFFFFFFFF // [31:0] fiq status after masking
#define REG_INTCPS_PENDING_FIQ1  0x000000BC // RO contains the FIQ status after masking
#define INTCPS_PENDING_FIQ1_PENDINGFIQ  0xFFFFFFFF // [31:0] fiq status after masking
#define REG_INTCPS_PENDING_FIQ2  0x000000DC // RO contains the FIQ status after masking
#define INTCPS_PENDING_FIQ2_PENDINGFIQ  0xFFFFFFFF // [31:0] fiq status after masking

#define INTCPS_ILR_RESERVED  0xFC // Reserved bits 31-8 & bit 1

#define REG_INTCPS_ILR0           0x00000100 // RW contains the priority for the interrupts and the FIQ/IRQ steering
#define REG_INTCPS_ILR1           0x00000104 //
#define REG_INTCPS_ILR2           0x00000108 //
#define REG_INTCPS_ILR3           0x0000010c //
#define REG_INTCPS_ILR4           0x00000110 //
#define REG_INTCPS_ILR5           0x00000114 //
#define REG_INTCPS_ILR6           0x00000118 //
#define REG_INTCPS_ILR7           0x0000011c //
#define REG_INTCPS_ILR8           0x00000120 //
#define REG_INTCPS_ILR9           0x00000124 //
#define REG_INTCPS_ILR10          0x00000128 //
#define REG_INTCPS_ILR11          0x0000012C //
#define REG_INTCPS_ILR12          0x00000130 //
#define REG_INTCPS_ILR13          0x00000134 //
#define REG_INTCPS_ILR14          0x00000138 //
#define REG_INTCPS_ILR15          0x0000013C //
#define REG_INTCPS_ILR16          0x00000140 //
#define REG_INTCPS_ILR17          0x00000144 //
#define REG_INTCPS_ILR18          0x00000148 //
#define REG_INTCPS_ILR19          0x0000014C //
#define REG_INTCPS_ILR20          0x00000150 //
#define REG_INTCPS_ILR21          0x00000154 //
#define REG_INTCPS_ILR22          0x00000158 //
#define REG_INTCPS_ILR23          0x0000015C //
#define REG_INTCPS_ILR24          0x00000160 //
#define REG_INTCPS_ILR25          0x00000164 //
#define REG_INTCPS_ILR26          0x00000168 //
#define REG_INTCPS_ILR27          0x0000016C //
#define REG_INTCPS_ILR28          0x00000170 //
#define REG_INTCPS_ILR29          0x00000174 //
#define REG_INTCPS_ILR30          0x00000178 //
#define REG_INTCPS_ILR31          0x0000017C //
#define REG_INTCPS_ILR32          0x00000180 //
#define REG_INTCPS_ILR33          0x00000184 //
#define REG_INTCPS_ILR34          0x00000188 //
#define REG_INTCPS_ILR35          0x0000018C //
#define REG_INTCPS_ILR36          0x00000190 //
#define REG_INTCPS_ILR37          0x00000194 //
#define REG_INTCPS_ILR38          0x00000198 //
#define REG_INTCPS_ILR39          0x0000019C //
#define REG_INTCPS_ILR40          0x000001A0 //
#define REG_INTCPS_ILR41          0x000001A4 //
#define REG_INTCPS_ILR42          0x000001A8 //
#define REG_INTCPS_ILR43          0x000001AC //
#define REG_INTCPS_ILR44          0x000001B0 //
#define REG_INTCPS_ILR45          0x000001B4 //
#define REG_INTCPS_ILR46          0x000001B8 //
#define REG_INTCPS_ILR47          0x000001BC //
#define REG_INTCPS_ILR48          0x000001C0 //
#define REG_INTCPS_ILR49          0x000001C4 //
#define REG_INTCPS_ILR50          0x000001C8 //
#define REG_INTCPS_ILR51          0x000001CC //
#define REG_INTCPS_ILR52          0x000001D0 //
#define REG_INTCPS_ILR53          0x000001D4 //
#define REG_INTCPS_ILR54          0x000001D8 //
#define REG_INTCPS_ILR55          0x000001DC //
#define REG_INTCPS_ILR56          0x000001E0 //
#define REG_INTCPS_ILR57          0x000001E4 //
#define REG_INTCPS_ILR58          0x000001E8 //
#define REG_INTCPS_ILR59          0x000001EC //
#define REG_INTCPS_ILR60          0x000001F0 //
#define REG_INTCPS_ILR61          0x000001F4 //
#define REG_INTCPS_ILR62          0x000001F8 //
#define REG_INTCPS_ILR63          0x000001FC //
#define REG_INTCPS_ILR64          0x00000200 //
#define REG_INTCPS_ILR65          0x00000204 //
#define REG_INTCPS_ILR66          0x00000208 //
#define REG_INTCPS_ILR67          0x0000020C //
#define REG_INTCPS_ILR68          0x00000210 //
#define REG_INTCPS_ILR69          0x00000214 //
#define REG_INTCPS_ILR70          0x00000218 //
#define REG_INTCPS_ILR71          0x0000021C //
#define REG_INTCPS_ILR72          0x00000220 //
#define REG_INTCPS_ILR73          0x00000224 //
#define REG_INTCPS_ILR74          0x00000228 //
#define REG_INTCPS_ILR75          0x0000022C //
#define REG_INTCPS_ILR76          0x00000230 //
#define REG_INTCPS_ILR77          0x00000234 //
#define REG_INTCPS_ILR78          0x00000238 //
#define REG_INTCPS_ILR79          0x0000023C //
#define REG_INTCPS_ILR80          0x00000240 //
#define REG_INTCPS_ILR81          0x00000244 //
#define REG_INTCPS_ILR82          0x00000248 //
#define REG_INTCPS_ILR83          0x0000024C //
#define REG_INTCPS_ILR84          0x00000250 //
#define REG_INTCPS_ILR85          0x00000254 //
#define REG_INTCPS_ILR86          0x00000258 //
#define REG_INTCPS_ILR87          0x0000025C //
#define REG_INTCPS_ILR88          0x00000260 //
#define REG_INTCPS_ILR89          0x00000264 //
#define REG_INTCPS_ILR90          0x00000268 //
#define REG_INTCPS_ILR91          0x0000026C //
#define REG_INTCPS_ILR92          0x00000270 //
#define REG_INTCPS_ILR93          0x00000274 //
#define REG_INTCPS_ILR94          0x00000278 //
#define REG_INTCPS_ILR95          0x0000027C //


void initIntc(void);

void intcReset(void);

/* top load function */
u32int loadIntc(device * dev, ACCESS_SIZE size, u32int address);


/* top store function */
void storeIntc(device * dev, ACCESS_SIZE size, u32int address, u32int value);

void maskInterrupt(u32int interruptNumber);

bool isGuestIrqMasked(u32int interruptNumber);

void unmaskInterrupt(u32int interruptNumber);

u32int getIrqNumber(void);

void setInterrupt(u32int irqNum);

void clearInterrupt(u32int irqNum);

void acknowledgeIrq(void);

bool isIrqPending(void);

bool isFiqPending(void);

void intcDumpRegisters(void);

u32int prioritySortIrqs(void);

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


#endif
