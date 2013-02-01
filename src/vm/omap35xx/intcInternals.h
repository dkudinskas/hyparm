#ifndef __VM__OMAP_35XX__INTC_INTERNALS_H__
#define __VM__OMAP_35XX__INTC_INTERNALS_H__

#define INTCPS_NR_OF_BANKS          3
#define INTCPS_INTERRUPTS_PER_BANK 32
#define INTCPS_NR_OF_INTERRUPTS    96

#define INTC_REVISION                                  0x00000040


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

#define REG_INTCPS_UNKNOWN_20    0x00000020

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

#endif /* __VM__OMAP_35XX__INTC_INTERNALS_H__ */
