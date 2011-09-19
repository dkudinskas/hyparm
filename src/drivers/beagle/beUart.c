#include "common/debug.h"
#include "common/memFunctions.h"

#include "drivers/beagle/beUart.h"


static inline u32int beGetUartNumber(u32int phyAddr);
static inline u32int beGetUartBaseAddr(u32int id) __attribute__((always_inline));

struct UartBackEnd * beUart[3];

void serialPuts(char * c)
{
  int index = 0;

  while (c[index] != '\0')
  {
    serialPutc(c[index]);
    index++;
  }
}

void serialPutc(char c)
{
  while ((beLoadUart(UART_LSR_REG, 3) & UART_LSR_TX_FIFO_E) == 0)
  {
    // do nothing
  }
  beStoreUart(UART_THR_REG, (u32int)c, 3);
}

char serialGetc()
{
  while ((beLoadUart(UART_LSR_REG, 3) & UART_LSR_RX_FIFO_E) == 0)
  {
    // do nothing
  }
  return (char)beLoadUart(UART_RHR_REG, 3);
}

char serialGetcAsync()
{
  return (char)beLoadUart(UART_RHR_REG, 3);
}


/*
 * initialize the device and put it in a harmless state
 */
void beUartInit(u32int uartid)
{
  u32int uID = uartid-1;
  beUart[uID] = (struct UartBackEnd*)mallocBytes(sizeof(struct UartBackEnd));
  if (beUart[uID] == 0)
  {
    DIE_NOW(0, "Failed to allocate UART backend.");
  }
  else
  {
    memset((void*)beUart[uID], 0x0, sizeof(struct UartBackEnd));
#ifdef BE_UART_DBG
    printf("Initializing UART%x backend at 0x%x\n", uartid, (u32int)beUart[uID]);
#endif
  }

  beUartReset(uartid);

  // disable all irqs
  beStoreUart(UART_IER_REG, 0, uartid);
  // put UART in disabled mode
  beStoreUart(UART_MDR1_REG, UART_MODE_DISABLED, uartid);

  // enable enhanced features
  beStoreUart(UART_LCR_REG, 0xBF, uartid);
  beStoreUart(UART_EFR_REG, UART_EFR_ENHANCED_EN, uartid);
  beStoreUart(UART_LCR_REG, 0x0, uartid);


  // clear, then disable FIFO's
  beStoreUart(UART_FCR_REG, UART_FCR_FIFO_EN, uartid);
  beStoreUart(UART_FCR_REG, UART_FCR_FIFO_EN |
                            UART_FCR_RX_FIFO_CLR |
                            UART_FCR_TX_FIFO_CLR, uartid);
  beStoreUart(UART_FCR_REG, 0, uartid);
  beUart[uID]->rxFifoSize = 0;
  beUart[uID]->txFifoSize = 0;

  // clear any pending events:
  // event bits auto clear?..
  beLoadUart(UART_RHR_REG, uartid);
  beLoadUart(UART_LSR_REG, uartid);
  beLoadUart(UART_MSR_REG, uartid);
  beLoadUart(UART_IIR_REG, uartid);

  beUart[uID]->loopback = FALSE;

  beUart[uID]->baseAddress = beGetUartBaseAddr(uartid);
  beUart[uID]->size = UART_SIZE;
}

/* just perform a software reset of the device */
void beUartReset(u32int uartid)
{
  u32int sysCtrl = beLoadUart(UART_SYSC_REG, uartid);
  sysCtrl |= UART_SYSC_REG_RESET;
  beStoreUart(UART_SYSC_REG, sysCtrl, uartid);

  while ( (beLoadUart(UART_SYSS_REG, uartid) & UART_SYSS_REG_RSTDONE) != UART_SYSS_REG_RSTDONE)
  {
    // do nothing
  }
  // reset complete
}


/*
 * prepare the device for operation: set up baud rates, interrupts, etc.
 */
void beUartStartup(u32int uartid, u32int baudRate)
{
  u32int divisor;

  // set nRTS output to active (low), Force DTR output to active (low)
  beStoreUart(UART_MCR_REG, UART_MCR_RTS | UART_MCR_DTR, uartid);

  // enable CTS wakeup event
  beStoreUart(UART_WER_REG, 1, uartid);

  // make sure mode is - disabled
  beStoreUart(UART_MDR1_REG, UART_MODE_DISABLED, uartid);

  // enable config mode and set 8bit word length
  beStoreUart(UART_LCR_REG, UART_LCR_DIV_EN | UART_LCR_CHAR_LEN_8, uartid);

  // divisor - zero
  beStoreUart(UART_DLL_REG, 0, uartid);
  beStoreUart(UART_DLH_REG, 0, uartid);

  // get back to operational mode
  beStoreUart(UART_LCR_REG, UART_LCR_CHAR_LEN_8, uartid);

  // clear TX and RX logic
  beStoreUart(UART_FCR_REG, UART_FCR_TX_FIFO_CLR | UART_FCR_RX_FIFO_CLR, uartid);

  // enable config mode
  beStoreUart(UART_LCR_REG, UART_LCR_DIV_EN | UART_LCR_CHAR_LEN_8, uartid);

  /*
   * The divisor for x16 mode can be calculated as follows:
   *
   * Operating frequency = 48000000
   * Divisor value = Operating frequency/(16x baud rate)
   *
   * See OMAP35x TRM rev. O, p. 2666 (UART/IrDA/CIR).
   */
  switch (baudRate)
  {
    case 500000:
      divisor = 6;
      break;
    default:
      /* baud rate 115200 */
      divisor = 26;
      break;
  }
  beStoreUart(UART_DLH_REG, (divisor >> 8) & 0xff, uartid);
  beStoreUart(UART_DLL_REG, divisor & 0xff, uartid);

  // get back to operational mode
  beStoreUart(UART_LCR_REG, UART_LCR_CHAR_LEN_8, uartid);

  // set mode to UARTx16
  beStoreUart(UART_MDR1_REG, UART_MODE_UARTx16, uartid);

  // enable RX interrupt
  beStoreUart(UART_IER_REG, UART_IER_RHR | UART_IER_LINE_ST, uartid);
}


u8int beLoadUart(u32int regOffs, u32int uartid)
{
  volatile u8int * regPtr = (u8int*)(beGetUartBaseAddr(uartid) | regOffs);
  return *regPtr;
}

void beStoreUart(u32int regOffs, u8int value, u32int uartid)
{
  volatile u8int * regPtr = (u8int*)(beGetUartBaseAddr(uartid) | regOffs);
  *regPtr = value;
}


void beUartDumpRegs(u32int uartid)
{
  // TODO
}

static inline u32int beGetUartBaseAddr(u32int id)
{
  switch(id)
  {
    case 1:
      return UART1;
    case 2:
      return UART2;
    case 3:
      return UART3;
    default:
      DIE_NOW(0, "beGetUartBaseAddr: invalid base id.");
  }
  return -1;
}

static inline u32int beGetUartNumber(u32int phyAddr)
{
  if ((phyAddr >= UART1) && (phyAddr < (UART1+UART_SIZE)))
  {
    return 1;
  }
  else if ((phyAddr >= UART2) && (phyAddr < (UART2+UART_SIZE)))
  {
    return 2;
  }
  else if ((phyAddr >= UART3) && (phyAddr < (UART3+UART_SIZE)))
  {
    return 3;
  }
  else
  {
    return -1;
  }
}
