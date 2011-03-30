#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"
#include "guestManager/guestExceptions.h"

#include "vm/omap35xx/uart.h"
#include "vm/omap35xx/intc.h"

#include "memoryManager/pageTable.h" // for getPhysicalAddress()

extern GCONTXT * getGuestContext(void);

static inline u32int getUartNumber(u32int phyAddr);
static inline u32int getUartBaseAddr(u32int id);

struct Uart * uart[3];

void initUart(u32int uartID)
{
  u32int uID = uartID-1;
  // init function: setup device, reset register values to defaults!
  uart[uID] = (struct Uart*)mallocBytes(sizeof(struct Uart));
  if (uart[uID] == 0)
  {
    DIE_NOW(0, "Failed to allocate uart.");
  }
  else
  {
    memset((void*)uart[uID], 0x0, sizeof(struct Uart));
#ifdef UART_DBG
    DEBUG_STRING("Initializing Uart");
    DEBUG_INT_NOZEROS(uartID);
    DEBUG_STRING(" at 0x");
    DEBUG_INT((u32int)uart[uID]);
    DEBUG_NEWLINE();
#endif
  }

  resetUart(uartID);
}

void resetUart(u32int uartID)
{
  u32int uID = uartID-1;
  // reset to default register values
  uart[uID]->loopback    = FALSE;
  int i = 0;
  for (i = 0; i < RX_FIFO_SIZE; i++)
  {
    uart[uID]->rxFifo[i] = 0;
  }
  uart[uID]->rxFifoPtr = 0;

  uart[uID]->dll         = 0x00000000;
  uart[uID]->rhr         = 0x00000000;
  uart[uID]->thr         = 0x00000000;
  uart[uID]->dlh         = 0x00000000;
  uart[uID]->ier         = 0x00000000;
  uart[uID]->iir         = 0x00000001; // no interrupt is pending bit set
  uart[uID]->fcr         = 0x00000000;
  uart[uID]->efr         = 0x00000000;
  uart[uID]->lcr         = 0x00000000;
  setUartMode(uartID);
  uart[uID]->mcr         = 0x00000000;
  uart[uID]->xon1        = 0x00000000;
  uart[uID]->lsr         = 0x00000060; // THR and shift register is empty
  uart[uID]->icr         = 0x00000000;
  uart[uID]->xon2        = 0x00000000;
  uart[uID]->msr         = 0x00000000;
  uart[uID]->tcr         = 0x0000000F;
  uart[uID]->xoff1       = 0x00000000;
  uart[uID]->spr         = 0x00000000;
  uart[uID]->tlr         = 0x00000000;
  uart[uID]->xoff2       = 0x00000000;
  uart[uID]->mdr1        = 0x00000007; // mode select: disable
  uart[uID]->mdr2        = 0x00000000;
  uart[uID]->uasr        = 0x00000000;
  uart[uID]->scr         = 0x00000000;
  uart[uID]->ssr         = 0x00000000;
  uart[uID]->mvr         = 0x00000047;
  uart[uID]->sysc        = 0x00000000;
  uart[uID]->syss        = 0x00000000;
  uart[uID]->wer         = 0x0000005F;
}


u32int loadUart(device * dev, ACCESS_SIZE size, u32int address)
{
  if (size != BYTE)
  {
    DIE_NOW(0, "UART: loadUart invalid access size - byte");
  }

  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int uID = getUartNumber(phyAddr);
  if (uID == 0)
  {
    DIE_NOW(0, "loadUart: invalid UART id.");
  }
  uID--; // array starts at 0
  u32int value = 0;
  u32int regOffs = phyAddr - getUartBaseAddr(uID+1);

  switch (regOffs)
  {
    case UART_DLL_REG: // also RHR, THR
    {
      if (getUartMode(uID+1) == operational)
      {
        // load RHR
        value = uartRxByte(uID+1);
      }
      else
      {
        // load DLL
#ifdef UART_DBG
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": load DLL value ");
        DEBUG_INT(uart[uID]->dll);
        DEBUG_NEWLINE();
#endif
        value = uart[uID]->dll;
      }
      break;
    }
    case UART_DLH_REG:
      if (getUartMode(uID+1) == operational)
      {
        // load IER
#ifdef UART_DBG
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": load IRQ enable register ");
        DEBUG_INT(uart[uID]->ier);
        DEBUG_NEWLINE();
#endif
        value = uart[uID]->ier;
      }
      else
      {
        // load DLH
#ifdef UART_DBG
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": load DLH register ");
        DEBUG_INT(uart[uID]->ier);
        DEBUG_NEWLINE();
#endif
        value = uart[uID]->dlh;
      }
      break;
    case UART_IIR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // load EFR
#ifdef UART_DBG
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": load enhanced feature register ");
        DEBUG_INT(uart[uID]->efr);
        DEBUG_NEWLINE();
#endif
        value = uart[uID]->efr;
      }
      else
      {
        // load IIR
#ifdef UART_DBG
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": load IRQ identification register ");
        DEBUG_INT(uart[uID]->iir);
        DEBUG_NEWLINE();
#endif
        value = uart[uID]->iir;
      }
      break;
    case UART_LCR_REG:
#ifdef UART_DBG
      DEBUG_STRING(dev->deviceName);
      DEBUG_STRING(": load line control reg ");
      DEBUG_INT(uart[uID]->iir);
      DEBUG_NEWLINE();
#endif
      value = uart[uID]->lcr;
      break;
    case UART_MCR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // load XON1_ADDR1
        DIE_NOW(0, "UART load XON1_ADDR1 unimplemented");
      }
      else
      {
        // load MCR
#ifdef UART_DBG
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": load modem control reg ");
        DEBUG_INT(uart[uID]->mcr);
        DEBUG_NEWLINE();
#endif
        value = uart[uID]->mcr;
      }
      break;
    case UART_LSR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // load XON2_ADDR2
        DIE_NOW(0, "UART load XON2_ADDR2 unimplemented");
      }
      else
      {
        // load LSR
#ifdef UART_DBG
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": load line status ");
        DEBUG_INT(uart[uID]->lsr);
        DEBUG_NEWLINE();
#endif
        value = uart[uID]->lsr;
      }
      break;
    case UART_MSR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // load TCR/XOFF1
        DIE_NOW(0, "UART load TCR/XOFF1 unimplemented");
      }
      else
      {
        // load MSR/TCR
        if ( ((uart[uID]->efr & UART_EFR_ENHANCED_EN) == 0) ||
             ((uart[uID]->mcr & UART_MCR_TCR_TLR) == 0) )
        {
          // sub-operational/sub-configA MSR_SPR mode, load modem status
          value = uart[uID]->msr;
        }
        else
        {
          // sub-operational/sub-configA TCR_TLR mode, load xmission control
          DIE_NOW(0, "UART store TCR unimplemented");
        }
      }
      break;
    case UART_SPR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // store TLR/XOFF2
        DIE_NOW(0, "UART load TLR/XOFF2 unimplemented");
      }
      else
      {
        // store SPR/TLR
        if ( ((uart[uID]->efr & UART_EFR_ENHANCED_EN) == 0) ||
             ((uart[uID]->mcr & UART_MCR_TCR_TLR) == 0) )
        {
          // sub-operational/sub-configA MSR_SPR mode, load scratchpad reg
          value = uart[uID]->spr;
        }
        else
        {
          // sub-operational/sub-configA TCR_TLR mode, store TLR reg
          DIE_NOW(0, "UART store TLR unimplemented");
        }
      }
      break;
    case UART_MDR1_REG:
      value = uart[uID]->mdr1;
      break;
    case UART_MDR2_REG:
    case UART_SFLSR_REG:
    case UART_RESUME_REG:
    case UART_SFREGL_REG:
    case UART_SFREGH_REG:
    case UART_UASR_REG:
    case UART_SCR_REG:
    case UART_SSR_REG:
    case UART_MVR_REG:
    case UART_SYSC_REG:
    case UART_SYSS_REG:
    case UART_WER_REG:
      dumpGuestContext(getGuestContext());
      DEBUG_STRING("loadUart");
      DEBUG_INT_NOZEROS(uID+1);
      DEBUG_STRING(" reg ");
      DEBUG_INT_NOZEROS(regOffs);
      DEBUG_NEWLINE();
      DIE_NOW(0, "UART: load from unimplemented register.");
    default:
      dumpGuestContext(getGuestContext());
      DEBUG_STRING("loadUart");
      DEBUG_INT_NOZEROS(uID+1);
      DEBUG_STRING(" reg ");
      DEBUG_INT_NOZEROS(regOffs);
      DEBUG_NEWLINE();
      DIE_NOW(0, "UART: load from undefined register.");
  } // switch ends
  
#ifdef UART_DBG
  DEBUG_STRING(dev->deviceName);
  DEBUG_STRING(": load from address ");
  DEBUG_INT(address);
  DEBUG_STRING(" reg ");
  DEBUG_INT_NOZEROS(regOffs);
  DEBUG_STRING(" value ");
  DEBUG_INT(value);
  DEBUG_NEWLINE(); 
#endif

 return value;
}

void storeUart(device * dev, ACCESS_SIZE size, u32int address, u32int value)
{
  if (size != BYTE)
  {
    DIE_NOW(0, "UART: storeUart invalid access size - byte");
  }

  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int uID = getUartNumber(phyAddr);
  if (uID == 0)
  {
    DIE_NOW(0, "storeUart: invalid UART id.");
  }
  uID--; // array starts at 0
  u32int regOffs = phyAddr - getUartBaseAddr(uID+1);

#ifdef UART_DBG
  DEBUG_STRING(dev->deviceName);
  DEBUG_STRING(": store to address ");
  DEBUG_INT(address);
  DEBUG_STRING(" reg ");
  DEBUG_INT_NOZEROS(regOffs);
  DEBUG_STRING(" value ");
  DEBUG_INT(value);
  DEBUG_NEWLINE(); 
#endif
  switch (regOffs)
  {
    case UART_DLL_REG:
    {
      if (getUartMode(uID+1) == operational)
      {
        // store THR
        uartTxByte((u8int)value, uID+1);
      }
      else
      {
        // store DLL
        // can only be written before sleep mode is enabled
        if ((uart[uID]->ier & UART_IER_SLEEP_MODE) != 0)
        {
          dumpGuestContext(gc);
          DIE_NOW(0, "UART writing DLL with sleep mode enabled!");
        }
        else
        {
#ifdef UART_DBG
          DEBUG_STRING(dev->deviceName);
          DEBUG_STRING(": storing DLL reg, 8bit LSB divisor value ");
          DEBUG_INT(value);
          DEBUG_NEWLINE(); 
#endif
          uart[uID]->dll = value;
        }
      }
      break;
    }
    case UART_DLH_REG:
      if (getUartMode(uID+1) == operational)
      {
        // store IER
        setIrqFlags(value, uID+1);
      }
      else
      {
        // store DLH
        // can only be written before sleep mode is enabled
        if ((uart[uID]->ier & UART_IER_SLEEP_MODE) != 0)
        {
          dumpGuestContext(gc);
          DIE_NOW(0, "UART writing DLH with sleep mode enabled!");
        }
        else
        {
#ifdef UART_DBG
          DEBUG_STRING(dev->deviceName);
          DEBUG_STRING(": store DLH MSB divisor value: ");
          DEBUG_INT(value);
          DEBUG_NEWLINE(); 
#endif
          uart[uID]->dlh = value;
        }
      }
      break;
    case UART_FCR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // store EFR
        uart[uID]->efr = value;
      }
      else
      {
        // store FCR
        if ( ((value & UART_FCR_FIFO_EN) == UART_FCR_FIFO_EN) &&
             ((uart[uID]->fcr & UART_FCR_FIFO_EN) == 0) )
        {
          // turning OFF RX/TX fifos.
#ifdef UART_DBG
          DEBUG_STRING("UART: warning: rx/tx fifos on!");
          DEBUG_NEWLINE();
#endif
        }
        else if ( ((uart[uID]->fcr & UART_FCR_FIFO_EN) == UART_FCR_FIFO_EN) &&
                  ((value & UART_FCR_FIFO_EN) == 0) )
        {
          // turning ON RX/TX fifos.
#ifdef UART_DBG
          DEBUG_STRING("UART: warning: rx/tx fifos off!");
          DEBUG_NEWLINE();
#endif
        }
        if ((value & UART_FCR_RX_FIFO_CLR) != 0)
        {
          // clear our RX FIFO.
          int i = 0;
          for (i = 0; i < RX_FIFO_SIZE; i++)
          {
            uart[uID]->rxFifo[i] = 0;
            uart[uID]->rxFifoPtr = 0;
            uart[uID]->rhr = 0;
            uart[uID]->lsr &= ~UART_LSR_RX_BI;
            uart[uID]->lsr &= ~UART_LSR_RX_FE;
            uart[uID]->lsr &= ~UART_LSR_RX_PE;
            uart[uID]->lsr &= ~UART_LSR_RX_OE;
            uart[uID]->lsr &= ~UART_LSR_RX_FIFO_E;
          }
        }
        // set new FCR value
        uart[uID]->fcr = value;
        // set 2 bits in IIR...
        u32int iirFcrMirrorBits = ((uart[uID]->fcr & UART_FCR_FIFO_EN) == 0) 
                                                  ? 0 : UART_IIR_FCR_MIRROR; 
        uart[uID]->iir = (uart[uID]->iir & ~UART_IIR_FCR_MIRROR) | iirFcrMirrorBits;
      }
      break;
    case UART_LCR_REG:
      uart[uID]->lcr = value;
      setUartMode(uID+1);
      break;
    case UART_MCR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // store XON1_ADDR1
        DIE_NOW(0, "UART store XON1_ADDR1 unimplemented");
      }
      else
      {
        // store MCR
        
        // must check if UART is being put to loopback mode
        if ( ((value & UART_MCR_LOOPBACK_EN) == UART_MCR_LOOPBACK_EN) &&
             ((uart[uID]->mcr & UART_MCR_LOOPBACK_EN) == 0) )
        {
          // putting UART in loopback mode!
          uart[uID]->loopback = TRUE;
          // adjust MSR register
#ifdef UART_DBG
          DEBUG_STRING("UART");
          DEBUG_INT_NOZEROS(uID+1);
          DEBUG_STRING(" loopback mode hack, magic number to MSR");
          DEBUG_NEWLINE();          
#endif
          uart[uID]->msr = 0x96;
        }
        else if ( ((uart[uID]->mcr & UART_MCR_LOOPBACK_EN) == UART_MCR_LOOPBACK_EN) &&
                  ((value & UART_MCR_LOOPBACK_EN) == 0) )
        {
          // switching off loopback mode!
#ifdef UART_DBG
          DEBUG_STRING("UART");
          DEBUG_INT_NOZEROS(uID+1);
          DEBUG_STRING(" loopback mode off.");
          DEBUG_NEWLINE();          
#endif
          uart[uID]->loopback = FALSE;
        }
        
        // bits [4:7] Can be written only when EFR_REG[4] = 1
        if ((uart[uID]->efr && UART_EFR_ENHANCED_EN) == 0)
        {
          // enchaned features disabled. only write bottom four bits
          uart[uID]->mcr = (uart[uID]->mcr & 0xFFFFFFE0) | (value & 0x1F);
        }
        else
        {
          // enchaned features enabled. write all 8 bits
          uart[uID]->mcr = (uart[uID]->mcr & 0xFFFFFFC0) | (value & 0x3F);
        }
      }
      break;
    case UART_LSR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // store XON2_ADDR2
        DIE_NOW(0, "UART store XON2_ADDR2 unimplemented");
      }
      else
      {
        /* OK, LINUX thinks there is some register here, but undocumented in SPRUF!
         * #define UART_ICR        0x05     Index Control Register 
         * http://lxr.linux.no/linux+v2.6.28.1/include/linux/serial_reg.h#L233 */
        uart[uID]->icr = value;
      }
      break;
    case UART_MSR_REG:
      DIE_NOW(0, "UART store MSR/TCR/XOFF1 unimplemented");
      break;
    case UART_SPR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // store TLR/XOFF2
        DIE_NOW(0, "UART store TLR/XOFF2 unimplemented");
      }
      else
      {
        // store SPR/TLR
        if ( ((uart[uID]->efr & UART_EFR_ENHANCED_EN) == 0) ||
             ((uart[uID]->mcr & UART_MCR_TCR_TLR) == 0) )
        {
          // sub-operational/sub-configA MSR_SPR mode, store scratchpad reg
          uart[uID]->spr = value;
        }
        else
        {
          // sub-operational/sub-configA TCR_TLR mode, store TLR reg
          DIE_NOW(0, "UART store TLR unimplemented");
        }
      }
      break;
    case UART_MDR1_REG:
    {
      DEBUG_STRING(dev->deviceName);
      switch (value & UART_MDR_MODE_SEL)
      {
        case UART_MODE_UARTx16:
          DEBUG_STRING(": warning - putting in UARTx16 mode!");
          DEBUG_NEWLINE();
          break;
        case UART_MODE_SIR:
          DEBUG_STRING(": warning - putting in SIR mode!");
          DEBUG_NEWLINE();
          break;
        case UART_MODE_UARTx16ABAUD:
          DEBUG_STRING(": warning - putting in UARTx16 autobaud mode!");
          DEBUG_NEWLINE();
          break;
        case UART_MODE_UARTx13:
          DEBUG_STRING(": warning - putting in UARTx13 mode!");
          DEBUG_NEWLINE();
          break;
        case UART_MODE_MIR:
          DEBUG_STRING(": warning - putting in MIR mode!");
          DEBUG_NEWLINE();
          break;
        case UART_MODE_FIR:
          DEBUG_STRING(": warning - putting in FIR mode!");
          DEBUG_NEWLINE();
          break;
        case UART_MODE_CIR:
          DEBUG_STRING(": warning - putting in CIR mode!");
          DEBUG_NEWLINE();
          break;
        case UART_MODE_DISABLED:
          DEBUG_STRING(": warning - putting in disabled state!");
          DEBUG_NEWLINE();
          break;
      }
      uart[uID]->mdr1 = value;
      break;
    }
    case UART_SCR_REG:
      uart[uID]->scr = value;
      break;
    case UART_SYSC_REG:
      if ((value & UART_SYSC_REG_RESET) == UART_SYSC_REG_RESET)
      {
        DEBUG_STRING(dev->deviceName);
        DEBUG_STRING(": soft reset.");
        DEBUG_NEWLINE();
        resetUart(uID+1);
      }
      uart[uID]->sysc = value;
      break;
    case UART_SYSS_REG:
      DEBUG_STRING(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (SYSS_REG)");
      break;
    case UART_UASR_REG:
      DEBUG_STRING(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (autobaud status)");
      break;
    case UART_SSR_REG:
      DEBUG_STRING(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (SSR)");
      break;
    case UART_MVR_REG:
      DEBUG_STRING(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (MVR)");
      break;
    case UART_MDR2_REG:
    case UART_SFLSR_REG:
    case UART_RESUME_REG:
    case UART_SFREGL_REG:
    case UART_SFREGH_REG:
    case UART_WER_REG:
      dumpGuestContext(getGuestContext());
      DEBUG_STRING("storeUart");
      DEBUG_INT_NOZEROS(uID+1);
      DEBUG_STRING(" reg ");
      DEBUG_INT_NOZEROS(regOffs);
      DEBUG_STRING(" value ");
      DEBUG_INT(value);
      DEBUG_NEWLINE();
      DIE_NOW(0, "UART: store to unimplemented register.");
    default:
      dumpGuestContext(getGuestContext());
      DEBUG_STRING("storeUart");
      DEBUG_INT_NOZEROS(uID+1);
      DEBUG_STRING(" reg ");
      DEBUG_INT_NOZEROS(regOffs);
      DEBUG_STRING(" value ");
      DEBUG_INT(value);
      DEBUG_NEWLINE();
      DIE_NOW(0, "UART: store to undefined register.");
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
      DIE_NOW(0, "getUartBaseAddr: invalid base id.");
  }
  return -1;
}

  
void setUartMode(u32int uartID)
{
#ifdef UART_DBG
  DEBUG_STRING("UART");
  DEBUG_INT_NOZEROS(uartID);
  DEBUG_STRING(": set mode to ");
#endif
  u32int uID = uartID-1;
  if ((uart[uID]->lcr & UART_LCR_DIV_EN) == UART_LCR_DIV_EN)
  {
    if ((uart[uID]->lcr & 0xBF) != 0xBF)
    {
      uart[uID]->mode = configA;
#ifdef UART_DBG
      DEBUG_STRING("configA");
#endif
    }
    else
    {
      uart[uID]->mode = configB;
#ifdef UART_DBG
      DEBUG_STRING("configB");
#endif
    }
  }
  else
  {
    uart[uID]->mode = operational;
#ifdef UART_DBG
      DEBUG_STRING("operational");
#endif
  }
#ifdef UART_DBG
  DEBUG_NEWLINE(); 
#endif
}

uartMode getUartMode(u32int uartID)
{
  return uart[uartID-1]->mode;
}


/*
 * function called when guest writes to THR register.
 * immediatelly transmits character to native serial
 * unless emulation configured to be in loopback mode
 */
void uartTxByte(u8int byte, u32int uartID)
{
  u32int uID = uartID - 1;
#ifdef UART_DBG
    DEBUG_STRING("uartTxByte: send '");
    DEBUG_CHAR((char)byte);
    DEBUG_STRING("' out via serial");
    DEBUG_NEWLINE(); 
#endif

  if (uart[uID]->loopback)
  {
#ifdef UART_DBG
    DEBUG_STRING("uartTxByte: in loopback mode - byte goes to RX FIFO.");
    DEBUG_NEWLINE(); 
#endif
    // do we have space in RX FIFO?
    if (uart[uID]->rxFifoPtr >= RX_FIFO_SIZE)
    {
      // can't receive anymore. drop value, set overrun LSR status bit
      uart[uID]->lsr |= UART_LSR_RX_OE;
    }
    else
    {
      // character goes straight back into RX
      if (uart[uID]->rxFifoPtr == 0)
      {
        // first char in FIFO, is also RHR
        uart[uID]->rhr = (u8int)(byte & 0xFF);
      }
      uart[uID]->rxFifo[uart[uID]->rxFifoPtr] = (u8int)(byte & 0xFF);
      // set LSR rx status bit to indicate RX data
      uart[uID]->lsr |= UART_LSR_RX_FIFO_E;
      // increment FIFO pointer
      uart[uID]->rxFifoPtr++;
    }
  }
  else
  {
    // don't need to adjust TX bits in LSR, as we will always
    // finish transmitting this character first, before going back to guest
    // therefore, the non-existant TX FIFO is trully always empty.
    DEBUG_CHAR((u8int)byte);
  }
}


/*
 * function called when guest reads from RHR register.
 * gets a byte out of RHR/RX FIFO, adjust IRQ bits and LSR
 */
u8int uartRxByte(u32int uartID)
{
  u32int uID = uartID - 1;
  u8int value = 0;

  // check LSR RX bit. if zero, return nil.
  if ((uart[uID]->lsr & UART_LSR_RX_FIFO_E) == 0)
  {
#ifdef UART_DBG
    DEBUG_STRING(dev->deviceName);
    DEBUG_STRING(": load RHR, but RX FIFO is empty! return 0");
    DEBUG_NEWLINE();
#endif
    value = 0;
  }
  else
  {
#ifdef UART_DBG
    DEBUG_STRING(dev->deviceName);
    DEBUG_STRING(": load RHR value ");
    DEBUG_INT(uart[uID]->rhr);
    DEBUG_NEWLINE();
#endif
    // there's stuff in the FIFO! get char from RHR
    value = uart[uID]->rhr;
    // adjust our fifo: all RX elements shift one position left
    int i = 0;
    for (i = 0; i < RX_FIFO_SIZE-1; i++)
    {
      uart[uID]->rxFifo[i] = uart[uID]->rxFifo[i+1]; 
    }
    // adjust FIFO ptr
    uart[uID]->rxFifoPtr--;
    // if we had previously overrun, clear LSR overrun bit
    uart[uID]->lsr &= ~UART_LSR_RX_OE;
    // set new RHR: if FIFO now empty, RHR zero and lsr bit cleared.
    // otherwise, FIFO char 0 is new RHR
    if (uart[uID]->rxFifoPtr == 0)
    {
      uart[uID]->rhr = 0; 
      uart[uID]->lsr &= ~UART_LSR_RX_FIFO_E;
      if ( (uart[uID]->ier & UART_IER_RHR) == UART_IER_RHR )
      {
        // RX IRQ was enabled. probably need to clear it.
        uart[uID]->iir = uart[uID]->iir | UART_IIR_IT_PENDING;
        uart[uID]->iir &= ~UART_IIR_IT_TYPE;
        if ( (uart[uID]->ier & UART_IER_THR) == UART_IER_THR )
        {
          // TX fifo is always empty, set new IRQ type
          uart[uID]->iir = uart[uID]->iir & ~UART_IIR_IT_PENDING;
          uart[uID]->iir &= ~UART_IIR_IT_TYPE;
          uart[uID]->iir = uart[uID]->iir | (UART_IIR_IT_TYPE_THR_IRQ << UART_IIR_IT_TYPE_SHAMT);
          switch (uID)
          {
            case 0:
              throwInterrupt(UART1_IRQ);
              break;
            case 1:
              throwInterrupt(UART2_IRQ);
              break;
            case 2:
              throwInterrupt(UART3_IRQ);
              break;
            default:
              DIE_NOW(0, "store to uart: invalid uID.");
          } // switch ends
        } // THR irq
      } // RHR irq
    }
    else
    {
      uart[uID]->rhr = uart[uID]->rxFifo[0];
    }
  } // RX FIFO turned on
  return value;
}



/*
 * function called from outside, when native serial receives data.
 * data is stored in RHR and RX fifo.
 * if they are full, data is dropped and exception indicated.
 */
void uartPutRxByte(u8int byte, u32int uartID)
{
  u32int uID = uartID - 1;

#ifdef UART_DBG
  DEBUG_STRING("uartPutRxByte: ");
  DEBUG_CHAR((char)byte);
  DEBUG_STRING(", ");
  DEBUG_INT(uartID);
  DEBUG_STRING(")");
  DEBUG_NEWLINE();
#endif

  if ((uart[uID]->fcr & UART_FCR_FIFO_EN) == 0)
  {
    // FIFO's are disabled!
    // just receive to RHR
    uart[uID]->rhr = (u8int)(byte & 0xFF);
    // set LSR rx status bit to indicate RX data
    uart[uID]->lsr |= UART_LSR_RX_FIFO_E;

    // set RX IRQ
    if ((uart[uID]->ier & UART_IER_RHR) == UART_IER_RHR)
    {
#ifdef UART_DBG
      DEBUG_STRING("uartPutRxByte: RX IRQ unmasked. Raise with INTC!");
      DEBUG_NEWLINE();
#endif
      uart[uID]->iir = uart[uID]->iir & ~UART_IIR_IT_PENDING;
      uart[uID]->iir &= ~UART_IIR_IT_TYPE;
      uart[uID]->iir |= (UART_IIR_IT_TYPE_RHR_IRQ << UART_IIR_IT_TYPE_SHAMT);
      switch (uID)
      {
        case 0:
          throwInterrupt(UART1_IRQ);
          break;
        case 1:
          throwInterrupt(UART2_IRQ);
          break;
        case 2:
          throwInterrupt(UART3_IRQ);
          break;
        default:
          DIE_NOW(0, "store to uart: invalid uID.");
      } // siwtch ends
    } // line status error IRQ enabled in UART
  }
  else
  {
    // do we have space in RX FIFO?
    if (uart[uID]->rxFifoPtr >= RX_FIFO_SIZE)
    {
      // can't receive anymore. drop value, set overrun LSR status bit
      uart[uID]->lsr |= UART_LSR_RX_OE;
#ifdef UART_DBG
      DEBUG_STRING("uartPutRxByte: no space in RX fifo!");
      DEBUG_NEWLINE();
#endif
      
      // set RX line status error
      if ((uart[uID]->ier & UART_IER_LINE_ST) == UART_IER_LINE_ST)
      {
        uart[uID]->iir = uart[uID]->iir & ~UART_IIR_IT_PENDING;
        uart[uID]->iir &= ~UART_IIR_IT_TYPE;
        uart[uID]->iir |= (UART_IIR_IT_TYPE_RX_LS_ERR_IRQ << UART_IIR_IT_TYPE_SHAMT);
#ifdef UART_DBG
        DEBUG_STRING("uartPutRxByte: RX line error IRQ unmasked. Raise with INTC!");
        DEBUG_NEWLINE();
#endif
        switch (uID)
        {
          case 0:
            throwInterrupt(UART1_IRQ);
            break;
          case 1:
            throwInterrupt(UART2_IRQ);
            break;
          case 2:
            throwInterrupt(UART3_IRQ);
            break;
          default:
            DIE_NOW(0, "store to uart: invalid uID.");
        } // siwtch ends
      } // line status error IRQ enabled in UART
    }
    else
    {
      // character goes into RX path
      if (uart[uID]->rxFifoPtr == 0)
      {
        // first char in FIFO, is also RHR
        uart[uID]->rhr = (u8int)(byte & 0xFF);
      }
      uart[uID]->rxFifo[uart[uID]->rxFifoPtr] = (u8int)(byte & 0xFF);
      // set LSR rx status bit to indicate RX data
      uart[uID]->lsr |= UART_LSR_RX_FIFO_E;
      // increment FIFO pointer
      uart[uID]->rxFifoPtr++;
      
      // set RX IRQ
      if ((uart[uID]->ier & UART_IER_RHR) == UART_IER_RHR)
      {
#ifdef UART_DBG
        DEBUG_STRING("uartPutRxByte: RX IRQ unmasked. Raise with INTC!");
        DEBUG_NEWLINE();
#endif
        uart[uID]->iir = uart[uID]->iir & ~UART_IIR_IT_PENDING;
        uart[uID]->iir &= ~UART_IIR_IT_TYPE;
        uart[uID]->iir |= (UART_IIR_IT_TYPE_RHR_IRQ << UART_IIR_IT_TYPE_SHAMT);
        switch (uID)
        {
          case 0:
            throwInterrupt(UART1_IRQ);
            break;
          case 1:
            throwInterrupt(UART2_IRQ);
            break;
          case 2:
            throwInterrupt(UART3_IRQ);
            break;
          default:
            DIE_NOW(0, "store to uart: invalid uID.");
        } // siwtch ends
      } // line status error IRQ enabled in UART
    } // RX character
  } // FIFO's enabled

}

void setIrqFlags(u32int flags, u32int uartID)
{
  u32int uID = uartID-1;
  
  if (((uart[uID]->ier & UART_IER_THR) == 0) &&
      ((flags & UART_IER_THR) == UART_IER_THR))
  {
    // enabling TX IRQ!
    // TX fifo is always empty. raise IRQ
    uart[uID]->iir = uart[uID]->iir & ~UART_IIR_IT_PENDING;
    uart[uID]->iir &= ~UART_IIR_IT_TYPE;
    uart[uID]->iir = uart[uID]->iir | (UART_IIR_IT_TYPE_THR_IRQ << UART_IIR_IT_TYPE_SHAMT);
#ifdef UART_DBG
    DEBUG_STRING("setIrqFlags: enabling THR irq, set IIR to ");
    DEBUG_INT(uart[uID]->iir);
    DEBUG_NEWLINE();
#endif
    switch (uID)
    {
      case 0:
        throwInterrupt(UART1_IRQ);
        break;
      case 1:
        throwInterrupt(UART2_IRQ);
        break;
      case 2:
        throwInterrupt(UART3_IRQ);
        break;
      default:
        DIE_NOW(0, "store to uart: invalid uID.");
    }
  }


  if (((uart[uID]->ier & UART_IER_THR) == UART_IER_THR) &&
          ((flags & UART_IER_THR) == 0))
  {
    // THR irq was enabled, clear irq
    uart[uID]->iir = uart[uID]->iir | UART_IIR_IT_PENDING;
    uart[uID]->iir = uart[uID]->iir &~ UART_IIR_IT_TYPE;
#ifdef UART_DBG
    DEBUG_STRING("setIrqFlags: disabling THR irq, set IIR to ");
    DEBUG_INT(uart[uID]->iir);
    DEBUG_NEWLINE();
#endif
  }


  if (((uart[uID]->ier & UART_IER_RHR) == 0) &&
      ((flags & UART_IER_RHR) == UART_IER_RHR))
  {
    // Enabling RX interrupt!
    // if LSR has RX_FIFO_E bit set, set RX irq flag in IIR
    if ( (uart[uID]->lsr & UART_LSR_RX_FIFO_E) == UART_LSR_RX_FIFO_E)
    {
      uart[uID]->iir = uart[uID]->iir & ~UART_IIR_IT_PENDING;
      uart[uID]->iir &= ~UART_IIR_IT_TYPE;
      uart[uID]->iir = uart[uID]->iir | (UART_IIR_IT_TYPE_RHR_IRQ << UART_IIR_IT_TYPE_SHAMT);
      switch (uID)
      {
        case 0:
          throwInterrupt(UART1_IRQ);
          break;
        case 1:
          throwInterrupt(UART2_IRQ);
          break;
        case 2:
          throwInterrupt(UART3_IRQ);
          break;
        default:
          DIE_NOW(0, "store to uart: invalid uID.");
      }
    }
#ifdef UART_DBG
    DEBUG_STRING("setIrqFlags: enabling RX irq, set IIR to ");
    DEBUG_INT(uart[uID]->iir);
    DEBUG_NEWLINE();
#endif
  }


  if (((uart[uID]->ier & UART_IER_RHR) == UART_IER_RHR) &&
     ((flags & UART_IER_RHR) == 0))
  {
    // disabling RX IRQ! clear the flag
    uart[uID]->iir = uart[uID]->iir | UART_IIR_IT_PENDING;
    uart[uID]->iir = uart[uID]->iir &~ UART_IIR_IT_TYPE;
    
    if ( (flags & UART_IER_THR) == UART_IER_THR )
    {
      // TX fifo is always empty, set new IRQ type
      uart[uID]->iir = uart[uID]->iir & ~UART_IIR_IT_PENDING;
      uart[uID]->iir &= ~UART_IIR_IT_TYPE;
      uart[uID]->iir = uart[uID]->iir | (UART_IIR_IT_TYPE_THR_IRQ << UART_IIR_IT_TYPE_SHAMT);
      switch (uID)
      {
        case 0:
          throwInterrupt(UART1_IRQ);
          break;
        case 1:
          throwInterrupt(UART2_IRQ);
          break;
        case 2:
          throwInterrupt(UART3_IRQ);
          break;
        default:
          DIE_NOW(0, "store to uart: invalid uID.");
      }
    }
#ifdef UART_DBG
    DEBUG_STRING("setIrqFlags: disabling RHR irq, set IIR to ");
    DEBUG_INT(uart[uID]->iir);
    DEBUG_NEWLINE();
#endif
  }


  // bits [4:7] Can be written only when EFR_REG[4] = 1
  u32int writenValue = flags;
  if ((uart[uID]->efr && UART_EFR_ENHANCED_EN) == 0)
  {
    // enchaned features disabled. only write bottom four bits
    writenValue &= 0xF;
    uart[uID]->ier &= 0xFFFFFFF0;
  }
  else
  {
    // enchaned features enabled. write all 8 bits
    writenValue &= 0xFF;
    uart[uID]->ier &= 0xFFFFFF00;
  }
  uart[uID]->ier = writenValue;
#ifdef UART_DBG
    DEBUG_STRING("setIrqFlags: storing IRQ enable register: ");
    DEBUG_INT(uart[uID]->ier);
    DEBUG_NEWLINE();
#endif
 
  if ((flags & UART_IER_SLEEP_MODE) != 0)
  {
#ifdef UART_DBG
    DEBUG_STRING("setIrqFlags: UART");
    DEBUG_INT_NOZEROS(uID+1);
    DEBUG_STRING(": sent to sleep mode!");
    DEBUG_NEWLINE();
#endif
  }
  
  // if now all irq are clear/disabled, make sure INTC line is clear
  if ((uart[uID]->iir & UART_IIR_IT_PENDING) == UART_IIR_IT_PENDING)
  {
    // no IRQ pending!
    switch (uID)
    {
      case 0:
        clearInterrupt(UART1_IRQ);
        break;
      case 1:
        clearInterrupt(UART2_IRQ);
        break;
      case 2:
        clearInterrupt(UART3_IRQ);
        break;
      default:
        DIE_NOW(0, "store to uart: invalid uID.");
    }
  }
}
