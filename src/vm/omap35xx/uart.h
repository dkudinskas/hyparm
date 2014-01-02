#ifndef __VM__OMAP35XX__UART_H__
#define __VM__OMAP35XX__UART_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/types.h"


#define RX_FIFO_SIZE    64


/* possible modes of operation: operational, configA and configB. */
/* all modes have possible submodes to differentiate register access */
typedef enum UART_REG_ACCESS_MODE
{
  configA,
  configB,
  operational
} uartMode;

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


void initUart(virtualMachine *vm, u32int uartID) __cold__;
u32int loadUart(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr);
void storeUart(GCONTXT *context, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value);
void uartPutRxByte(GCONTXT *context, u8int byte, u32int uardID);

#endif /* __VM__OMAP35XX__UART_H__ */
