#include "common/debug.h"
#include "common/stddef.h"
#include "common/stdlib.h"
#include "common/string.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/intc.h"
#include "vm/omap35xx/uart.h"
#include "vm/omap35xx/uartInternals.h"


static inline u32int getUartNumber(u32int phyAddr);
static inline u32int getUartBaseAddr(u32int id);
static void setIrqFlags(GCONTXT *context, struct Uart *uart, u32int id, u32int flags);
static void setUartMode(struct Uart *uart);
static void resetUart(struct Uart *uart);
static u8int uartRxByte(GCONTXT *context, struct Uart *uart, u32int id);
static void uartTxByte(struct Uart *uart, u8int byte);


void initUart(virtualMachine *vm, u32int uartID)
{
  u32int uID = uartID - 1;
  // init function: setup device, reset register values to defaults!
  vm->uart[uID] = (struct Uart *)calloc(1, sizeof(struct Uart));
  if (vm->uart[uID] == NULL)
  {
    DIE_NOW(NULL, "Failed to allocate uart.");
  }

  DEBUG(VP_OMAP_35XX_UART, "Initializing Uart%x at %p" EOL, uartID, vm->uart[uID]);
  resetUart(vm->uart[uID]);
}

static void resetUart(struct Uart *uart)
{
  // reset to default register values
  uart->loopback = FALSE;
  int i;
  for (i = 0; i < RX_FIFO_SIZE; i++)
  {
    uart->rxFifo[i] = 0;
  }
  uart->rxFifoPtr = 0;

  uart->dll         = 0x00000000;
  uart->rhr         = 0x00000000;
  uart->thr         = 0x00000000;
  uart->dlh         = 0x00000000;
  uart->ier         = 0x00000000;
  uart->iir         = 0x00000001; // no interrupt is pending bit set
  uart->fcr         = 0x00000000;
  uart->efr         = 0x00000000;
  uart->lcr         = 0x00000000;
  setUartMode(uart);
  uart->mcr         = 0x00000000;
  uart->xon1        = 0x00000000;
  uart->lsr         = 0x00000060; // THR and shift register is empty
  uart->icr         = 0x00000000;
  uart->xon2        = 0x00000000;
  uart->msr         = 0x00000000;
  uart->tcr         = 0x0000000F;
  uart->xoff1       = 0x00000000;
  uart->spr         = 0x00000000;
  uart->tlr         = 0x00000000;
  uart->xoff2       = 0x00000000;
  uart->mdr1        = 0x00000007; // mode select: disable
  uart->mdr2        = 0x00000000;
  uart->uasr        = 0x00000000;
  uart->scr         = 0x00000000;
  uart->ssr         = 0x00000000;
  uart->mvr         = 0x00000047;
  uart->sysc        = 0x00000000;
  uart->syss        = 0x00000000;
  uart->wer         = 0x0000005F;
}


u32int loadUart(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr)
{
  /*
   * Normally access size is a word but linux accesses in halfword and byte size too.
   */
#ifndef CONFIG_GUEST_ANDROID
  if (size != BYTE)
  {
    DIE_NOW(NULL, "UART: loadUart invalid access size - byte");
  }
#endif

  u32int uID = getUartNumber(phyAddr);
  if (uID == 0)
  {
    DIE_NOW(NULL, "loadUart: invalid UART id.");
  }

  u32int value = 0;
  u32int regOffs = phyAddr - getUartBaseAddr(uID);

  uID--; // array starts at 0
  struct Uart* uart = context->vm.uart[uID];

  switch (regOffs)
  {
    case UART_DLL_REG: // also RHR, THR
    {
      if (uart->mode == operational)
      {
        // load RHR
        value = uartRxByte(context, uart, uID);
      }
      else
      {
        // load DLL
        DEBUG(VP_OMAP_35XX_UART, "%s: load DLL value %#.8x" EOL, dev->deviceName, uart->dll);
        value = uart->dll;
      }
      break;
    }
    case UART_DLH_REG:
    {
      if (uart->mode == operational)
      {
        // load IER
        DEBUG(VP_OMAP_35XX_UART, "%s: load IRQ enable register %#.8x" EOL, dev->deviceName,
            uart->ier);
        value = uart->ier;
      }
      else
      {
        // load DLH
        DEBUG(VP_OMAP_35XX_UART, "%s: load DLH register %#.8x" EOL, dev->deviceName,
            uart->ier);
        value = uart->dlh;
      }
      break;
    }
    case UART_IIR_REG:
    {
      if (uart->mode == configB)
      {
        // load EFR
        DEBUG(VP_OMAP_35XX_UART, "%s: load enhanced feature register %#.8x" EOL, dev->deviceName,
            uart->efr);
        value = uart->efr;
      }
      else
      {
        // load IIR
        DEBUG(VP_OMAP_35XX_UART, "%s: load IRQ identification register %#.8x" EOL, dev->deviceName,
            uart->iir);
        value = uart->iir;
      }
      break;
    }
    case UART_LCR_REG:
    {
      DEBUG(VP_OMAP_35XX_UART, "%s: load line control reg %#.8x" EOL, dev->deviceName,
          uart->iir);
      value = uart->lcr;
      break;
    }
    case UART_MCR_REG:
    {
      if (uart->mode == configB)
      {
        // load XON1_ADDR1
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      else
      {
        // load MCR
        DEBUG(VP_OMAP_35XX_UART, "%s: load modem control reg %#.8x" EOL, dev->deviceName,
            uart->mcr);
        value = uart->mcr;
      }
      break;
    }
    case UART_LSR_REG:
    {
      if (uart->mode == configB)
      {
        // load XON2_ADDR2
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      else
      {
        // load LSR
        DEBUG(VP_OMAP_35XX_UART, "%s: load line status %#.8x" EOL, dev->deviceName, uart->lsr);
        value = uart->lsr;
      }
      break;
    }
    case UART_MSR_REG:
    {
      if (uart->mode == configB)
      {
        // load TCR/XOFF1
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      else
      {
        // load MSR/TCR
        if ( ((uart->efr & UART_EFR_ENHANCED_EN) == 0) ||
             ((uart->mcr & UART_MCR_TCR_TLR) == 0) )
        {
          // sub-operational/sub-configA MSR_SPR mode, load modem status
          value = uart->msr;
        }
        else
        {
          // sub-operational/sub-configA TCR_TLR mode, load xmission control
          DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
        }
      }
      break;
    }
    case UART_SPR_REG:
    {
      if (uart->mode == configB)
      {
        // store TLR/XOFF2
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      else
      {
        // store SPR/TLR
        if ( ((uart->efr & UART_EFR_ENHANCED_EN) == 0) ||
             ((uart->mcr & UART_MCR_TCR_TLR) == 0) )
        {
          // sub-operational/sub-configA MSR_SPR mode, load scratchpad reg
          value = uart->spr;
        }
        else
        {
          // sub-operational/sub-configA TCR_TLR mode, store TLR reg
          DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
        }
      }
      break;
    }
    case UART_MDR1_REG:
    {
      value = uart->mdr1;
      break;
    }
    case UART_SYSC_REG:
    {
      value = uart->sysc;
      break;
    }
    case UART_MDR2_REG:
    case UART_SFLSR_REG:
    case UART_RESUME_REG:
    case UART_SFREGL_REG:
    case UART_SFREGH_REG:
    case UART_UASR_REG:
    case UART_SCR_REG:
    case UART_SSR_REG:
    case UART_MVR_REG:
    case UART_SYSS_REG:
    case UART_WER_REG:
      printf("loadUart%x reg %#x" EOL, uID+1, regOffs);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    default:
      printf("loadUart%x reg %#x" EOL, uID+1, regOffs);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends

  DEBUG(VP_OMAP_35XX_UART, "%s: load from address %#.8x reg %#x value %#.8x" EOL, dev->deviceName,
      virtAddr, regOffs, value);

  return value;
}


void storeUart(GCONTXT *context, device *dev, ACCESS_SIZE size, u32int virtAddr, u32int phyAddr, u32int value)
{
  /*
   * Normally access size is a word but linux accesses in halfword and byte size too.
   */
#ifndef CONFIG_GUEST_ANDROID
  if (size != BYTE)
  {
    DIE_NOW(NULL, "UART: storeUart invalid access size - byte");
  }
#endif

  u32int uID = getUartNumber(phyAddr);
  if (uID == 0)
  {
    DIE_NOW(NULL, "storeUart: invalid UART id.");
  }

  u32int regOffs = phyAddr - getUartBaseAddr(uID);

  uID--; // array starts at 0
  struct Uart* uart = context->vm.uart[uID];

  DEBUG(VP_OMAP_35XX_UART, "%s: store to address %#.8x reg %#x value %#.8x" EOL, dev->deviceName,
      virtAddr, regOffs, value);
  switch (regOffs)
  {
    case UART_DLL_REG:
    {
      if (uart->mode == operational)
      {
        // store THR
        uartTxByte(uart, (u8int)value);
      }
      else
      {
        // store DLL
        // can only be written before sleep mode is enabled
        if ((uart->ier & UART_IER_SLEEP_MODE) != 0)
        {
          DIE_NOW(NULL, "UART writing DLL with sleep mode enabled!");
        }
        else
        {
          DEBUG(VP_OMAP_35XX_UART, "%s: storing DLL reg, 8bit LSB divisor value %#.8x" EOL,
              dev->deviceName, value);
          uart->dll = value;
        }
      }
      break;
    }
    case UART_DLH_REG:
    {
      if (uart->mode == operational)
      {
        // store IER
        setIrqFlags(context, uart, uID, value);
      }
      else
      {
        // store DLH
        // can only be written before sleep mode is enabled
        if ((uart->ier & UART_IER_SLEEP_MODE) != 0)
        {
          DIE_NOW(NULL, "UART writing DLH with sleep mode enabled!");
        }
        else
        {
          DEBUG(VP_OMAP_35XX_UART, "%s: store DLH MSB divisor value: %#.8x" EOL, dev->deviceName,
              value);
          uart->dlh = value;
        }
      }
      break;
    }
    case UART_FCR_REG:
    {
      if (uart->mode == configB)
      {
        // store EFR
        uart->efr = value;
      }
      else
      {
        // store FCR
        if ( ((value & UART_FCR_FIFO_EN) == UART_FCR_FIFO_EN) &&
             ((uart->fcr & UART_FCR_FIFO_EN) == 0) )
        {
          // turning OFF RX/TX fifos.
          DEBUG(VP_OMAP_35XX_UART, "UART: warning: rx/tx fifos on!" EOL);
        }
        else if ( ((uart->fcr & UART_FCR_FIFO_EN) == UART_FCR_FIFO_EN) &&
                  ((value & UART_FCR_FIFO_EN) == 0) )
        {
          // turning ON RX/TX fifos.
          DEBUG(VP_OMAP_35XX_UART, "UART: warning: rx/tx fifos off!" EOL);
        }
        if ((value & UART_FCR_RX_FIFO_CLR) != 0)
        {
          // clear our RX FIFO.
          int i = 0;
          for (i = 0; i < RX_FIFO_SIZE; i++)
          {
            uart->rxFifo[i] = 0;
            uart->rxFifoPtr = 0;
            uart->rhr = 0;
            uart->lsr &= ~UART_LSR_RX_BI;
            uart->lsr &= ~UART_LSR_RX_FE;
            uart->lsr &= ~UART_LSR_RX_PE;
            uart->lsr &= ~UART_LSR_RX_OE;
            uart->lsr &= ~UART_LSR_RX_FIFO_E;
          }
        }
        // set new FCR value
        uart->fcr = value;
        // set 2 bits in IIR...
        u32int iirFcrMirrorBits = ((uart->fcr & UART_FCR_FIFO_EN) == 0)
                                                  ? 0 : UART_IIR_FCR_MIRROR;
        uart->iir = (uart->iir & ~UART_IIR_FCR_MIRROR) | iirFcrMirrorBits;
      }
      break;
    }
    case UART_LCR_REG:
    {
      uart->lcr = value;
      setUartMode(uart);
      break;
    }
    case UART_MCR_REG:
    {
      if (uart->mode == configB)
      {
        // store XON1_ADDR1
        uart->xon1 = value;
      }
      else
      {
        // store MCR

        // must check if UART is being put to loopback mode
        if ( ((value & UART_MCR_LOOPBACK_EN) == UART_MCR_LOOPBACK_EN) &&
             ((uart->mcr & UART_MCR_LOOPBACK_EN) == 0) )
        {
          // putting UART in loopback mode!
          uart->loopback = TRUE;
          // adjust MSR register
          DEBUG(VP_OMAP_35XX_UART, "UART%x loopback mode hack, magic number to MSR" EOL, uID+1);
          uart->msr = 0x96;
        }
        else if ( ((uart->mcr & UART_MCR_LOOPBACK_EN) == UART_MCR_LOOPBACK_EN) &&
                  ((value & UART_MCR_LOOPBACK_EN) == 0) )
        {
          // switching off loopback mode!
          DEBUG(VP_OMAP_35XX_UART, "UART%x  loopback mode off." EOL, uID+1);
          uart->loopback = FALSE;
        }

        // bits [4:7] Can be written only when EFR_REG[4] = 1
        if (!(uart->efr & UART_EFR_ENHANCED_EN))
        {
          // enchaned features disabled. only write bottom four bits
          uart->mcr = (uart->mcr & 0xFFFFFFE0) | (value & 0x1F);
        }
        else
        {
          // enchaned features enabled. write all 8 bits
          uart->mcr = (uart->mcr & 0xFFFFFFC0) | (value & 0x3F);
        }
      }
      break;
    }
    case UART_LSR_REG:
    {
      if (uart->mode == configB)
      {
        // store XON2_ADDR2
        uart->xon2 = value;
      }
      else
      {
        /* OK, LINUX thinks there is some register here, but undocumented in SPRUF!
         * #define UART_ICR        0x05     Index Control Register
         * http://lxr.linux.no/linux+v2.6.28.1/include/linux/serial_reg.h#L233 */
        uart->icr = value;
      }
      break;
    }
    case UART_MSR_REG:
    {
      if (uart->mode == configB
          && (!(uart->efr & UART_EFR_ENHANCED_EN) || !(uart->efr & UART_EFR_ENHANCED_EN)))
      {
        // store XOFF1
        uart->xoff1 = value & U8INT_MAX;
      }
      else
      {
        // store TCR
        uart->tcr = value & U8INT_MAX;
      }
      break;
    }
    case UART_SPR_REG:
    {
      if (uart->mode == configB)
      {
        // store TLR/XOFF2
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
      }
      else
      {
        // store SPR/TLR
        if ( ((uart->efr & UART_EFR_ENHANCED_EN) == 0) ||
             ((uart->mcr & UART_MCR_TCR_TLR) == 0) )
        {
          // sub-operational/sub-configA MSR_SPR mode, store scratchpad reg
          uart->spr = value;
        }
        else
        {
          // sub-operational/sub-configA TCR_TLR mode, store TLR reg
          DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
        }
      }
      break;
    }
    case UART_MDR1_REG:
    {
      DEBUG(VP_OMAP_35XX_UART, "%s", dev->deviceName);
      switch (value & UART_MDR_MODE_SEL)
      {
        case UART_MODE_UARTx16:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in UARTx16 mode!" EOL);
          break;
        case UART_MODE_SIR:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in SIR mode!" EOL);
          break;
        case UART_MODE_UARTx16ABAUD:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in UARTx16 autobaud mode!" EOL);
          break;
        case UART_MODE_UARTx13:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in UARTx13 mode!" EOL);
          break;
        case UART_MODE_MIR:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in MIR mode!" EOL);
          break;
        case UART_MODE_FIR:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in FIR mode!" EOL);
          break;
        case UART_MODE_CIR:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in CIR mode!" EOL);
          break;
        case UART_MODE_DISABLED:
          DEBUG(VP_OMAP_35XX_UART, ": warning - putting in disabled state!" EOL);
          break;
      }
      uart->mdr1 = value;
      break;
    }
    case UART_SCR_REG:
    {
      uart->scr = value;
      break;
    }
    case UART_SYSC_REG:
    {
      if ((value & UART_SYSC_REG_RESET) == UART_SYSC_REG_RESET)
      {
        printf("%s: soft reset" EOL, dev->deviceName);
        resetUart(uart);
      }
      uart->sysc = value;
      break;
    }
    case UART_SYSS_REG:
    {
      printf("%s", dev->deviceName);
      DIE_NOW(NULL, " storing to R/O register (SYSS_REG)");
      break;
    }
    case UART_UASR_REG:
    {
      printf("%s", dev->deviceName);
      DIE_NOW(NULL, " storing to R/O register (autobaud status)");
      break;
    }
    case UART_SSR_REG:
    {
      printf("%s", dev->deviceName);
      DIE_NOW(NULL, " storing to R/O register (SSR)");
      break;
    }
    case UART_MVR_REG:
    {
      printf("%s", dev->deviceName);
      DIE_NOW(NULL, " storing to R/O register (MVR)");
      break;
    }
    case UART_WER_REG:
    {
      if (uart->wer != value)
      {
        printf("Unimplemented store to UART WER" EOL);
      }
      break;
    }
    case UART_MDR2_REG:
    case UART_SFLSR_REG:
    case UART_RESUME_REG:
    case UART_SFREGL_REG:
    case UART_SFREGH_REG:
      printf("storeUart%x reg %#x value %#.8x" EOL, uID+1, regOffs, value);
      DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
    default:
      printf("storeUart%x reg %#x value %#.8x" EOL, uID+1, regOffs, value);
      DIE_NOW(NULL, ERROR_NO_SUCH_REGISTER);
  } // switch ends
}

static inline u32int getUartNumber(u32int phyAddr)
{
  if ((phyAddr >= UART1) && (phyAddr < (UART1+UART1_SIZE)))
  {
    return 1;
  }
  else if ((phyAddr >= UART2) && (phyAddr < (UART2+UART2_SIZE)))
  {
    return 2;
  }
  else if ((phyAddr >= UART3) && (phyAddr < (UART3+UART3_SIZE)))
  {
    return 3;
  }
  else
  {
    return -1;
  }
}

static inline u32int getUartBaseAddr(u32int id)
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
      DIE_NOW(NULL, "getUartBaseAddr: invalid base id.");
  }
  return -1;
}


static void setUartMode(struct Uart *uart)
{
  if ((uart->lcr & UART_LCR_DIV_EN) == UART_LCR_DIV_EN)
  {
    if ((uart->lcr & 0xBF) != 0xBF)
    {
      uart->mode = configA;
      DEBUG(VP_OMAP_35XX_UART, "UART%p: set mode to configA" EOL, uart);
    }
    else
    {
      uart->mode = configB;
      DEBUG(VP_OMAP_35XX_UART, "UART%p: set mode to configB" EOL, uart);
    }
  }
  else
  {
    uart->mode = operational;
    DEBUG(VP_OMAP_35XX_UART, "UART%p: set mode to operational" EOL, uart);
  }
}



/*
 * function called when guest writes to THR register.
 * immediatelly transmits character to native serial
 * unless emulation configured to be in loopback mode
 */
static void uartTxByte(struct Uart *uart, u8int byte)
{
  DEBUG(VP_OMAP_35XX_UART, "uartTxByte: send ASCII %#x" EOL, byte);

  if (uart->loopback)
  {
    DEBUG(VP_OMAP_35XX_UART, "uartTxByte: in loopback mode - byte goes to RX FIFO." EOL);
    // do we have space in RX FIFO?
    if (uart->rxFifoPtr >= RX_FIFO_SIZE)
    {
      // can't receive anymore. drop value, set overrun LSR status bit
      uart->lsr |= UART_LSR_RX_OE;
    }
    else
    {
      // character goes straight back into RX
      if (uart->rxFifoPtr == 0)
      {
        // first char in FIFO, is also RHR
        uart->rhr = (u8int)(byte & 0xFF);
      }
      uart->rxFifo[uart->rxFifoPtr] = (u8int)(byte & 0xFF);
      // set LSR rx status bit to indicate RX data
      uart->lsr |= UART_LSR_RX_FIFO_E;
      // increment FIFO pointer
      uart->rxFifoPtr++;
    }
  }
  else
  {
    // don't need to adjust TX bits in LSR, as we will always
    // finish transmitting this character first, before going back to guest
    // therefore, the non-existant TX FIFO is trully always empty.
    printf("%c", (s32int)byte);
  }
}


/*
 * function called when guest reads from RHR register.
 * gets a byte out of RHR/RX FIFO, adjust IRQ bits and LSR
 */
static u8int uartRxByte(GCONTXT *context, struct Uart *uart, u32int id)
{
  u8int value = 0;

  // check LSR RX bit. if zero, return nil.
  if ((uart->lsr & UART_LSR_RX_FIFO_E) == 0)
  {
    DEBUG(VP_OMAP_35XX_UART, "UART%x: load RHR, but RX FIFO is empty! return 0" EOL, id + 1);
    value = 0;
  }
  else
  {
    DEBUG(VP_OMAP_35XX_UART, "UART%x: load RHR value %#.8x" EOL, id + 1, uart->rhr);
    // there's stuff in the FIFO! get char from RHR
    value = uart->rhr;
    // adjust our fifo: all RX elements shift one position left
    int i = 0;
    for (i = 0; i < RX_FIFO_SIZE-1; i++)
    {
      uart->rxFifo[i] = uart->rxFifo[i+1];
    }
    // adjust FIFO ptr
    uart->rxFifoPtr--;
    // if we had previously overrun, clear LSR overrun bit
    uart->lsr &= ~UART_LSR_RX_OE;
    // set new RHR: if FIFO now empty, RHR zero and lsr bit cleared.
    // otherwise, FIFO char 0 is new RHR
    if (uart->rxFifoPtr == 0)
    {
      uart->rhr = 0;
      uart->lsr &= ~UART_LSR_RX_FIFO_E;
      if ( (uart->ier & UART_IER_RHR) == UART_IER_RHR )
      {
        // RX IRQ was enabled. probably need to clear it.
        uart->iir = uart->iir | UART_IIR_IT_PENDING;
        uart->iir &= ~UART_IIR_IT_TYPE;
        if ( (uart->ier & UART_IER_THR) == UART_IER_THR )
        {
          // TX fifo is always empty, set new IRQ type
          uart->iir = uart->iir & ~UART_IIR_IT_PENDING;
          uart->iir &= ~UART_IIR_IT_TYPE;
          uart->iir = uart->iir | (UART_IIR_IT_TYPE_THR_IRQ << UART_IIR_IT_TYPE_SHAMT);
          switch (id)
          {
            case 0:
              throwInterrupt(context, UART1_IRQ);
              break;
            case 1:
              throwInterrupt(context, UART2_IRQ);
              break;
            case 2:
              throwInterrupt(context, UART3_IRQ);
              break;
            default:
              DIE_NOW(NULL, "store to uart: invalid uID.");
          } // switch ends
        } // THR irq
      } // RHR irq
    }
    else
    {
      uart->rhr = uart->rxFifo[0];
    }
  } // RX FIFO turned on
  return value;
}



/*
 * function called from outside, when native serial receives data.
 * data is stored in RHR and RX fifo.
 * if they are full, data is dropped and exception indicated.
 */
void uartPutRxByte(GCONTXT *context, u8int byte, u32int uartID)
{
  u32int uID = uartID - 1;
  struct Uart* uart = context->vm.uart[uID];

  DEBUG(VP_OMAP_35XX_UART, "UART%x: uartPutRxByte: %c" EOL, uartID, byte);

  if ((uart->fcr & UART_FCR_FIFO_EN) == 0)
  {
    // FIFO's are disabled!
    // just receive to RHR
    uart->rhr = (u8int)(byte & 0xFF);
    // set LSR rx status bit to indicate RX data
    uart->lsr |= UART_LSR_RX_FIFO_E;

    // set RX IRQ
    if ((uart->ier & UART_IER_RHR) == UART_IER_RHR)
    {
      DEBUG(VP_OMAP_35XX_UART, "uartPutRxByte: RX IRQ unmasked. Raise with INTC!" EOL);
      uart->iir = uart->iir & ~UART_IIR_IT_PENDING;
      uart->iir &= ~UART_IIR_IT_TYPE;
      uart->iir |= (UART_IIR_IT_TYPE_RHR_IRQ << UART_IIR_IT_TYPE_SHAMT);
      switch (uID)
      {
        case 0:
          throwInterrupt(context, UART1_IRQ);
          break;
        case 1:
          throwInterrupt(context, UART2_IRQ);
          break;
        case 2:
          throwInterrupt(context, UART3_IRQ);
          break;
        default:
          DIE_NOW(NULL, "store to uart: invalid uID.");
      } // siwtch ends
    } // line status error IRQ enabled in UART
  }
  else
  {
    // do we have space in RX FIFO?
    if (uart->rxFifoPtr >= RX_FIFO_SIZE)
    {
      // can't receive anymore. drop value, set overrun LSR status bit
      uart->lsr |= UART_LSR_RX_OE;
      DEBUG(VP_OMAP_35XX_UART, "uartPutRxByte: no space in RX fifo!" EOL);

      // set RX line status error
      if ((uart->ier & UART_IER_LINE_ST) == UART_IER_LINE_ST)
      {
        uart->iir = uart->iir & ~UART_IIR_IT_PENDING;
        uart->iir &= ~UART_IIR_IT_TYPE;
        uart->iir |= (UART_IIR_IT_TYPE_RX_LS_ERR_IRQ << UART_IIR_IT_TYPE_SHAMT);
        DEBUG(VP_OMAP_35XX_UART, "uartPutRxByte: RX line error IRQ unmasked. Raise with INTC!" EOL);
        switch (uID)
        {
          case 0:
            throwInterrupt(context, UART1_IRQ);
            break;
          case 1:
            throwInterrupt(context, UART2_IRQ);
            break;
          case 2:
            throwInterrupt(context, UART3_IRQ);
            break;
          default:
            DIE_NOW(NULL, "store to uart: invalid uID.");
        } // siwtch ends
      } // line status error IRQ enabled in UART
    }
    else
    {
      // character goes into RX path
      if (uart->rxFifoPtr == 0)
      {
        // first char in FIFO, is also RHR
        uart->rhr = (u8int)(byte & 0xFF);
      }
      uart->rxFifo[uart->rxFifoPtr] = (u8int)(byte & 0xFF);
      // set LSR rx status bit to indicate RX data
      uart->lsr |= UART_LSR_RX_FIFO_E;
      // increment FIFO pointer
      uart->rxFifoPtr++;

      // set RX IRQ
      if ((uart->ier & UART_IER_RHR) == UART_IER_RHR)
      {
        DEBUG(VP_OMAP_35XX_UART, "uartPutRxByte: RX IRQ unmasked. Raise with INTC!" EOL);
        uart->iir = uart->iir & ~UART_IIR_IT_PENDING;
        uart->iir &= ~UART_IIR_IT_TYPE;
        uart->iir |= (UART_IIR_IT_TYPE_RHR_IRQ << UART_IIR_IT_TYPE_SHAMT);
        switch (uID)
        {
          case 0:
            throwInterrupt(context, UART1_IRQ);
            break;
          case 1:
            throwInterrupt(context, UART2_IRQ);
            break;
          case 2:
            throwInterrupt(context, UART3_IRQ);
            break;
          default:
            DIE_NOW(NULL, "store to uart: invalid uID.");
        } // siwtch ends
      } // line status error IRQ enabled in UART
    } // RX character
  } // FIFO's enabled

}

static void setIrqFlags(GCONTXT *context, struct Uart *uart, u32int id, u32int flags)
{
  if (((uart->ier & UART_IER_THR) == 0) &&
      ((flags & UART_IER_THR) == UART_IER_THR))
  {
    // enabling TX IRQ!
    // TX fifo is always empty. raise IRQ
    uart->iir = uart->iir & ~UART_IIR_IT_PENDING;
    uart->iir &= ~UART_IIR_IT_TYPE;
    uart->iir = uart->iir | (UART_IIR_IT_TYPE_THR_IRQ << UART_IIR_IT_TYPE_SHAMT);
    DEBUG(VP_OMAP_35XX_UART, "setIrqFlags: enabling THR irq, set IIR to %#x" EOL, uart->iir);
    switch (id)
    {
      case 0:
        throwInterrupt(context, UART1_IRQ);
        break;
      case 1:
        throwInterrupt(context, UART2_IRQ);
        break;
      case 2:
        throwInterrupt(context, UART3_IRQ);
        break;
      default:
        DIE_NOW(NULL, "store to uart: invalid uID.");
    }
  }


  if (((uart->ier & UART_IER_THR) == UART_IER_THR) &&
          ((flags & UART_IER_THR) == 0))
  {
    // THR irq was enabled, clear irq
    uart->iir = uart->iir | UART_IIR_IT_PENDING;
    uart->iir = uart->iir &~ UART_IIR_IT_TYPE;
    DEBUG(VP_OMAP_35XX_UART, "setIrqFlags: disabling THR irq, set IIR to %#x" EOL, uart->iir);
  }


  if (!(uart->ier & UART_IER_RHR) && (flags & UART_IER_RHR))
  {
    // Enabling RX interrupt!
    // if LSR has RX_FIFO_E bit set, set RX irq flag in IIR
    if ((uart->lsr & UART_LSR_RX_FIFO_E) == UART_LSR_RX_FIFO_E)
    {
      uart->iir = uart->iir & ~UART_IIR_IT_PENDING;
      uart->iir &= ~UART_IIR_IT_TYPE;
      uart->iir = uart->iir | (UART_IIR_IT_TYPE_RHR_IRQ << UART_IIR_IT_TYPE_SHAMT);
      switch (id)
      {
        case 0:
          throwInterrupt(context, UART1_IRQ);
          break;
        case 1:
          throwInterrupt(context, UART2_IRQ);
          break;
        case 2:
          throwInterrupt(context, UART3_IRQ);
          break;
        default:
          DIE_NOW(NULL, "store to uart: invalid uID.");
      }
    }
    DEBUG(VP_OMAP_35XX_UART, "setIrqFlags: enabling RX irq, set IIR to %#x" EOL, uart->iir);
  }

  if (((uart->ier & UART_IER_RHR) == UART_IER_RHR) &&
     ((flags & UART_IER_RHR) == 0))
  {
    // disabling RX IRQ! clear the flag
    uart->iir = uart->iir | UART_IIR_IT_PENDING;
    uart->iir = uart->iir &~ UART_IIR_IT_TYPE;

    if ( (flags & UART_IER_THR) == UART_IER_THR )
    {
      // TX fifo is always empty, set new IRQ type
      uart->iir = uart->iir & ~UART_IIR_IT_PENDING;
      uart->iir &= ~UART_IIR_IT_TYPE;
      uart->iir = uart->iir | (UART_IIR_IT_TYPE_THR_IRQ << UART_IIR_IT_TYPE_SHAMT);
      switch (id)
      {
        case 0:
          throwInterrupt(context, UART1_IRQ);
          break;
        case 1:
          throwInterrupt(context, UART2_IRQ);
          break;
        case 2:
          throwInterrupt(context, UART3_IRQ);
          break;
        default:
          DIE_NOW(NULL, "store to uart: invalid uID.");
      }
    }
    DEBUG(VP_OMAP_35XX_UART, "setIrqFlags: disabling RHR irq, set IIR to %#x" EOL, uart->iir);
  }


  // bits [4:7] Can be written only when EFR_REG[4] = 1
  u32int writenValue = flags;
  if (!(uart->efr & UART_EFR_ENHANCED_EN))
  {
    // enchaned features disabled. only write bottom four bits
    writenValue &= 0xF;
    uart->ier &= 0xFFFFFFF0;
  }
  else
  {
    // enchaned features enabled. write all 8 bits
    writenValue &= 0xFF;
    uart->ier &= 0xFFFFFF00;
  }
  uart->ier = writenValue;
  DEBUG(VP_OMAP_35XX_UART, "setIrqFlags: storing IRQ enable register: %#x" EOL, uart->ier);

  if ((flags & UART_IER_SLEEP_MODE) != 0)
  {
    DEBUG(VP_OMAP_35XX_UART, "setIrqFlags: UART%x: : sent to sleep mode!" EOL, id + 1);
  }

  // if now all irq are clear/disabled, make sure INTC line is clear
  if ((uart->iir & UART_IIR_IT_PENDING) == UART_IIR_IT_PENDING)
  {
    // no IRQ pending!
    switch (id)
    {
      case 0:
        clearInterrupt(context, UART1_IRQ);
        break;
      case 1:
        clearInterrupt(context, UART2_IRQ);
        break;
      case 2:
        clearInterrupt(context, UART3_IRQ);
        break;
      default:
        DIE_NOW(NULL, "store to uart: invalid uID.");
    }
  }
}
