#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestManager/guestContext.h"

#include "hardware/serial.h"
#include "hardware/uart.h"

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
    serial_putstring("Initializing Uart");
    serial_putint_nozeros(uartID);
    serial_putstring(" at 0x");
    serial_putint((u32int)uart[uID]);
    serial_newline();
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
  uart[uID]->iir         = 0x00000001;
  uart[uID]->fcr         = 0x00000000;
  uart[uID]->efr         = 0x00000000;
  uart[uID]->lcr         = 0x00000000;
  setUartMode(uartID);
  uart[uID]->mcr         = 0x00000000;
  uart[uID]->xon1        = 0x00000000;
  uart[uID]->lsr         = 0x00000060;
  uart[uID]->icr         = 0x00000000;
  uart[uID]->xon2        = 0x00000000;
  uart[uID]->msr         = 0x00000000;
  uart[uID]->tcr         = 0x0000000F;
  uart[uID]->xoff1       = 0x00000000;
  uart[uID]->spr         = 0x00000000;
  uart[uID]->tlr         = 0x00000000;
  uart[uID]->xoff2       = 0x00000000;
  uart[uID]->mdr1        = 0x00000007;
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
    case UART_DLL_REG:
    {
      if (getUartMode(uID+1) == operational)
      {
        // load RHR
        // check LSR RX bit. if zero, return nil.
        if ((uart[uID]->lsr & UART_LSR_RX_FIFO_E) == 0)
        {
          value = 0;
        }
        else
        {
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
          }
          else
          {
            uart[uID]->rhr = uart[uID]->rxFifo[0];
          }
        }
      }
      else
      {
        // load DLL
        value = uart[uID]->dll;
      }
      break;
    }
    case UART_DLH_REG:
      if (getUartMode(uID+1) == operational)
      {
        // load IER
        value = uart[uID]->ier;
      }
      else
      {
        // load DLH
        value = uart[uID]->dlh;
      }
      break;
    case UART_IIR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // load EFR
        value = uart[uID]->efr;
      }
      else
      {
        // load IIR
        value = uart[uID]->iir;
      }
      break;
    case UART_LCR_REG:
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
        value = uart[uID]->lsr;
      }
      break;
    case UART_MSR_REG:
      if (getUartMode(uID+1) == configB)
      {
        // store TCR/XOFF1
        DIE_NOW(0, "UART store TCR/XOFF1 unimplemented");
      }
      else
      {
        // store MSR/TCR
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
      DIE_NOW(0, "UART load MSR/TCR/XOFF1 unimplemented");
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
      DIE_NOW(0, "UART load SPR/TLR/XOFF2 unimplemented");
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
      serial_putstring("loadUart");
      serial_putint_nozeros(uID+1);
      serial_putstring(" reg ");
      serial_putint_nozeros(regOffs);
      serial_newline();
      DIE_NOW(0, "UART: load from unimplemented register.");
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("loadUart");
      serial_putint_nozeros(uID+1);
      serial_putstring(" reg ");
      serial_putint_nozeros(regOffs);
      serial_newline();
      DIE_NOW(0, "UART: load from undefined register.");
  } // switch ends
  
#ifdef UART_DBG
  serial_putstring(dev->deviceName);
  serial_putstring(": load from address ");
  serial_putint(address);
  serial_putstring(" reg ");
  serial_putint_nozeros(regOffs);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline(); 
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
  serial_putstring(dev->deviceName);
  serial_putstring(": store to address ");
  serial_putint(address);
  serial_putstring(" reg ");
  serial_putint_nozeros(regOffs);
  serial_putstring(" value ");
  serial_putint(value);
  serial_newline(); 
#endif
  switch (regOffs)
  {
    case UART_DLL_REG:
    {
      if (getUartMode(uID+1) == operational)
      {
        // store THR
        if (uart[uID]->loopback)
        {
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
              uart[uID]->rhr = (u8int)(value & 0xFF);
            }
            uart[uID]->rxFifo[uart[uID]->rxFifoPtr] = (u8int)(value & 0xFF);
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
          serial_putchar((u8int)value);
        }
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
          uart[uID]->dll = value;
        }
      }
      break;
    }
    case UART_DLH_REG:
      if (getUartMode(uID+1) == operational)
      {
        // store IER
        // bits [4:7] Can be written only when EFR_REG[4] = 1
        if ((uart[uID]->efr && UART_EFR_ENHANCED_EN) == 0)
        {
          // enchaned features disabled. only write bottom four bits
          uart[uID]->ier = (uart[uID]->ier & 0xFFFFFFF0) | (value & 0xF);
        }
        else
        {
          // enchaned features enabled. write all 8 bits
          uart[uID]->ier = (uart[uID]->ier & 0xFFFFFF00) | (value & 0xFF);
        }
        if ((value & UART_IER_SLEEP_MODE) != 0)
        {
#ifdef UART_DBG
          serial_putstring("UART");
          serial_putint_nozeros(uID+1);
          serial_putstring(": sent to sleep mode!");
          serial_newline();
#endif
        }
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
          serial_putstring("UART: warning: rx/tx fifos on!");
          serial_newline();
#endif
        }
        else if ( ((uart[uID]->fcr & UART_FCR_FIFO_EN) == UART_FCR_FIFO_EN) &&
                  ((value & UART_FCR_FIFO_EN) == 0) )
        {
          // turning ON RX/TX fifos.
#ifdef UART_DBG
          serial_putstring("UART: warning: rx/tx fifos off!");
          serial_newline();
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
          serial_putstring("UART");
          serial_putint_nozeros(uID+1);
          serial_putstring(" loopback mode hack, magic number to MSR");
          serial_newline();          
#endif
          uart[uID]->msr = 0x96;
        }
        else if ( ((uart[uID]->mcr & UART_MCR_LOOPBACK_EN) == UART_MCR_LOOPBACK_EN) &&
                  ((value & UART_MCR_LOOPBACK_EN) == 0) )
        {
          // switching off loopback mode!
#ifdef UART_DBG
          serial_putstring("UART");
          serial_putint_nozeros(uID+1);
          serial_putstring(" loopback mode off.");
          serial_newline();          
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
      serial_putstring(dev->deviceName);
      switch (value & UART_MDR_MODE_SEL)
      {
        case UART_MODE_UARTx16:
          serial_putstring(": warning - putting in UARTx16 mode!");
          serial_newline();
          break;
        case UART_MODE_SIR:
          serial_putstring(": warning - putting in SIR mode!");
          serial_newline();
          break;
        case UART_MODE_UARTx16ABAUD:
          serial_putstring(": warning - putting in UARTx16 autobaud mode!");
          serial_newline();
          break;
        case UART_MODE_UARTx13:
          serial_putstring(": warning - putting in UARTx13 mode!");
          serial_newline();
          break;
        case UART_MODE_MIR:
          serial_putstring(": warning - putting in MIR mode!");
          serial_newline();
          break;
        case UART_MODE_FIR:
          serial_putstring(": warning - putting in FIR mode!");
          serial_newline();
          break;
        case UART_MODE_CIR:
          serial_putstring(": warning - putting in CIR mode!");
          serial_newline();
          break;
        case UART_MODE_DISABLED:
          serial_putstring(": warning - putting in disabled state!");
          serial_newline();
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
        serial_putstring(dev->deviceName);
        serial_putstring(": soft reset.");
        serial_newline();
        resetUart(uID+1);
      }
      uart[uID]->sysc = value;
      break;
    case UART_SYSS_REG:
      serial_putstring(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (SYSS_REG)");
      break;
    case UART_UASR_REG:
      serial_putstring(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (autobaud status)");
      break;
    case UART_SSR_REG:
      serial_putstring(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (SSR)");
      break;
    case UART_MVR_REG:
      serial_putstring(dev->deviceName);
      DIE_NOW(0, " storing to R/O register (MVR)");
      break;
    case UART_MDR2_REG:
    case UART_SFLSR_REG:
    case UART_RESUME_REG:
    case UART_SFREGL_REG:
    case UART_SFREGH_REG:
    case UART_WER_REG:
      dumpGuestContext(getGuestContext());
      serial_putstring("storeUart");
      serial_putint_nozeros(uID+1);
      serial_putstring(" reg ");
      serial_putint_nozeros(regOffs);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      DIE_NOW(0, "UART: store to unimplemented register.");
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("storeUart");
      serial_putint_nozeros(uID+1);
      serial_putstring(" reg ");
      serial_putint_nozeros(regOffs);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
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
  serial_putstring("UART");
  serial_putint_nozeros(uartID);
  serial_putstring(": set mode to ");
#endif
  u32int uID = uartID-1;
  if ((uart[uID]->lcr & UART_LCR_DIV_EN) == UART_LCR_DIV_EN)
  {
    if ((uart[uID]->lcr & 0xBF) != 0xBF)
    {
      uart[uID]->mode = configA;
#ifdef UART_DBG
      serial_putstring("configA");
#endif
    }
    else
    {
      uart[uID]->mode = configB;
#ifdef UART_DBG
      serial_putstring("configB");
#endif
    }
  }
  else
  {
    uart[uID]->mode = operational;
#ifdef UART_DBG
      serial_putstring("operational");
#endif
  }
#ifdef UART_DBG
  serial_newline(); 
#endif
}

uartMode getUartMode(u32int uartID)
{
  return uart[uartID-1]->mode;
}
