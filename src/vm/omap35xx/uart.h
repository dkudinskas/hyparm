#ifndef __HARDWARE__UART_H__
#define __HARDWARE__UART_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


// all register accesses are in words, 
// all registers are 16 bits wide
// all registers have top byte reserved (only bottom 8 bits are used)
// register offsets and bit definitions
#define UART_DLL_REG          0x00000000 // divisor latches low, RW
#define UART_RHR_REG          0x00000000 // receive holding register R/O
#define UART_THR_REG          0x00000000 // transmit holding register, W/O
#define UART_DLH_REG          0x00000004 // divisor latches high, R/W
#define UART_IER_REG          0x00000004 // interrupt enable register, R/W
#define UART_IER_RHR            0x00000001 // RHR irq and timeout irq
#define UART_IER_THR            0x00000002 // THR irq
#define UART_IER_LINE_ST        0x00000004 // receiver line status irq
#define UART_IER_MODEM_ST       0x00000008 // modem status irq
#define UART_IER_SLEEP_MODE     0x00000010 // en/disable sleep mode
#define UART_IER_XOFF           0x00000020 // xoff irq
#define UART_IER_RST            0x00000040 // reset irq
#define UART_IER_CTS            0x00000080 // CTS interrupt
#define UART_IIR_REG          0x00000008 // interrupt identification register, R/O
#define UART_IIR_FCR_MIRROR     0x000000C0 // Mirror FCR[0] on both bits 
#define UART_IIR_IT_TYPE        0x0000003E // one of seven interrupt types
#define UART_IIR_IT_TYPE_MODEM_IRQ       0x0
#define UART_IIR_IT_TYPE_THR_IRQ         0x1
#define UART_IIR_IT_TYPE_RHR_IRQ         0x2
#define UART_IIR_IT_TYPE_RX_LS_ERR_IRQ   0x3
#define UART_IIR_IT_TYPE_RX_TIMEOUT_IRQ  0x6
#define UART_IIR_IT_TYPE_XOFF_IRQ        0x8
#define UART_IIR_IT_TYPE_CTS_IRQ         0x10
#define UART_IIR_IT_TYPE_SHAMT           0x1
#define UART_IIR_IT_PENDING     0x00000001 // interrupt pending bit
#define UART_FCR_REG          0x00000008 // FIFO control register, W/O
#define UART_FCR_RX_FIFO_TRIG   0x0000000C // RX FIFO trigger level
#define UART_FCR_TX_FIFO_TRIG   0x00000003 // TX FIFO trigger level 
#define UART_FCR_DMA_MODE       0x00000008 // set DMA on (1) or off (0)
#define UART_FCR_TX_FIFO_CLR    0x00000004 // clear TX FIFO, reset counter to 0
#define UART_FCR_RX_FIFO_CLR    0x00000002 // clear RX FIFO, reset counter to 0
#define UART_FCR_FIFO_EN        0x00000001 // enable TX/RX FIFO's 
#define UART_EFR_REG          0x00000008 // enhanced feature set, R/W
#define UART_EFR_AUTO_CTS_EN    0x00000080 // auto-cts enable, uart only
#define UART_EFR_AUTO_RTS_EN    0x00000040 // auto-rts enable, uart only
#define UART_EFR_SPEC_CHAR      0x00000020 // special character detect enable (cmp with XOFF2)
#define UART_EFR_ENHANCED_EN    0x00000010 // enhanced functions write enable
#define UART_EFR_SW_FLOW_CTRL   0x0000000F // software flow control
#define UART_LCR_REG          0x0000000C // line control, R/W
#define UART_LCR_DIV_EN         0x00000080 // divisor latch enable
#define UART_LCR_BREAK_EN       0x00000040 // break control bit
#define UART_LCR_PARITY_TYPE2   0x00000020 // forced parity format
#define UART_LCR_PARITY_TYPE1   0x00000010 // 1: even 0: odd parity is generated
#define UART_LCR_PARITY_EN      0x00000008 // parity bit enable
#define UART_LCR_NB_STOP        0x00000004 // number of stop bits
#define UART_LCR_CHAR_LEN       0x00000003 // word length to be xmitted
#define UART_MCR_REG          0x00000010 // modem control, R/W
#define UART_MCR_TCR_TLR        0x00000040 // enable access to TCR/TLR registers
#define UART_MCR_XON_EN         0x00000020 // enable XON any function
#define UART_MCR_LOOPBACK_EN    0x00000010 // enable internal loopback mode
#define UART_MCR_CD_STS_CH      0x00000008 // in loopback force nDCD input low and IRQ outputs to inactive
#define UART_MCR_RI_STS_CH      0x00000004 // In loopback, forces nRI input active (low)
#define UART_MCR_RTS            0x00000002 // Force nRTS output to active (low)
#define UART_MCR_DTR            0x00000001 // Force DTR output (used in loopback) to active (low)
#define UART_XON1_ADDR1_REG   0x00000010 // XON1 character, R/W
#define UART_LSR_REG          0x00000014 // line status, R/O
#define UART_LSR_RX_FIFO_STS    0x00000080 // break indication in RX
#define UART_LSR_TX_SR_E        0x00000040 // THR (and fifo) and shift registers are empty
#define UART_LSR_TX_FIFO_E      0x00000020 // THR (and fifo) is empty
#define UART_LSR_RX_BI          0x00000010 // RX break detected
#define UART_LSR_RX_FE          0x00000008 // RX frame error detected
#define UART_LSR_RX_PE          0x00000004 // RX parity error detected
#define UART_LSR_RX_OE          0x00000002 // RX overrun error detected
#define UART_LSR_RX_FIFO_E      0x00000001 // 0: no data in RX fifo, 1: at least one char in RX FIFO
#define UART_XON2_ADDR2_REG   0x00000014 // XON2 character, R/W
#define UART_ICR_REG          0x00000014 // W/O ? UNDOCUMENTED reg, found in linux 
#define UART_MSR_REG          0x00000018 // modem status, R/O
#define UART_MSR_NCD_STS        0x00000080 // 
#define UART_MSR_NRI_STS        0x00000040 // 
#define UART_MSR_NDSR_STS       0x00000020 // 
#define UART_MSR_NCTS_STS       0x00000010 // 
#define UART_MSR_DCD_STS        0x00000008 // 
#define UART_MSR_RI_STS         0x00000004 // 
#define UART_MSR_DSR_STS        0x00000002 // 
#define UART_MSR_CTS_STS        0x00000001 //
#define UART_TCR_REG          0x00000018 // transmisison control, R/W
#define UART_XOFF1_REG        0x00000018 // XOFF1 character
#define UART_SPR_REG          0x0000001C // scratchpad, R/W 
#define UART_TLR_REG          0x0000001C // trigger level, R/W
#define UART_XOFF2_REG        0x0000001C // XOFF2 character
#define UART_MDR1_REG         0x00000020 // mode definition register 1, R/W
#define UART_MDR_FEND_MODE       0x00000008
#define UART_MDR_SIP_MODE        0x00000004
#define UART_MDR_SCT             0x00000002
#define UART_MDR_SET_TX_IR       0x00000001
#define UART_MDR_IR_SLEEP        0x00000008
#define UART_MDR_MODE_SEL        0x00000007
#define UART_MODE_UARTx16               0x0
#define UART_MODE_SIR                   0x1
#define UART_MODE_UARTx16ABAUD          0x2
#define UART_MODE_UARTx13               0x3
#define UART_MODE_MIR                   0x4
#define UART_MODE_FIR                   0x5
#define UART_MODE_CIR                   0x6
#define UART_MODE_DISABLED              0x7
#define UART_MDR2_REG         0x00000024 // mode definition register 2, R/W
#define UART_SFLSR_REG        0x00000028 // R/O
#define UART_TXFLL_REG        0x00000028 // W/O
#define UART_RESUME_REG       0x0000002c // R/O
#define UART_TXFLH_REG        0x0000002c // W/O
#define UART_SFREGL_REG       0x00000030 // R/O
#define UART_RXFLL_REG        0x00000030 // W/O
#define UART_SFREGH_REG       0x00000034 // R/O
#define UART_RXFLH_REG        0x00000034 // W/O
#define UART_UASR_REG         0x00000038 // UART autobauding status, R/O autobaud mode
#define UART_UASR_REG_PARITY_TYPE    0x000000c0
#define UART_UASR_REG_BIT_BY_CHAR    0x00000020
#define UART_UASR_REG_SPEED          0x0000001f
#define UART_SCR_REG          0x00000040 // Supplementary control register, R/W
#define UART_SCR_REG_RX_TRIG_GRANU1  0x00000080
#define UART_SCR_REG_TX_TRIG_GRANU1  0x00000040
#define UART_SCR_REG_RX_CTS_WU_EN    0x00000010
#define UART_SCR_REG_TX_EMPTY_CTL_IT 0x00000008
#define UART_SCR_REG_DMA_MODE_2      0x00000006
#define UART_SCR_REG_DMA_MODE_CTL    0x00000001
#define UART_SSR_REG          0x00000044 // Supplementary status register, R/O
#define UART_SSR_REG_RX_CTS      0x00000002
#define UART_SSR_REG_TXFIFO_FULL 0x00000001
#define UART_MVR_REG          0x00000050 // Module version register, R/O
#define UART_MVR_REG_REVNR       0x000000FF
#define UART_SYSC_REG         0x00000054 // System configuration register, R/W
#define UART_SYSC_REG_IDLEMODE   0x00000018
#define UART_SYSC_REG_ENAWAKEUP  0x00000004
#define UART_SYSC_REG_RESET      0x00000002
#define UART_SYSC_REG_AUTOIDLE   0x00000001
#define UART_SYSS_REG         0x00000058 // System status register, R/O
#define UART_SYSS_REG_RSTDONE    0x00000001
#define UART_WER_REG          0x0000005C // Wake-up enable register, R/W


#define RX_FIFO_SIZE    64

/* possible modes of operation: operational, configA and configB. */
/* all modes have possible submodes to differentiate register access */
enum UART_REG_ACCESS_MODE
{
  configA,
  configB,
  operational
};

typedef enum UART_REG_ACCESS_MODE uartMode;

void initUart(virtualMachine *vm, u32int uartID) __cold__;

u32int loadUart(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);

void storeUart(device * dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);

void uartPutRxByte(u8int byte, u32int uardID);

struct Uart
{
  uartMode mode;
  bool loopback;
  u8int rxFifo[RX_FIFO_SIZE];
  u32int rxFifoPtr;
  u32int dll;
  u32int rhr;
  u32int thr;
  u32int dlh;
  u32int ier;
  u32int iir;
  u32int fcr;
  u32int efr;
  u32int lcr;
  u32int mcr;
  u32int xon1;
  u32int lsr;
  u32int icr;
  u32int xon2;
  u32int msr;
  u32int tcr;
  u32int xoff1;
  u32int spr;
  u32int tlr;
  u32int xoff2;
  u32int mdr1;
  u32int mdr2;
  u32int uasr;
  u32int scr;
  u32int ssr;
  u32int mvr;
  u32int sysc;
  u32int syss;
  u32int wer;
};

#endif
