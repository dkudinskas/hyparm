#ifndef __UART_H__
#define __UART_H__

#include "types.h"
#include "serial.h"
#include "hardwareLibrary.h"


// uncomment me to enable debug : #define UART_DBG

// register offsets and bit definitions
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

void initUart(u32int uartID);

void resetUart(u32int uartID);

u32int loadUart(device * dev, ACCESS_SIZE size, u32int address);

void storeUart(device * dev, ACCESS_SIZE size, u32int address, u32int value);

struct Uart
{
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
