#include "uart.h"
#include "pageTable.h" // for getPhysicalAddress()
#include "guestContext.h"
#include "memFunctions.h"

extern GCONTXT * getGuestContext(void);

struct Uart * uart[3];

void initUart(u32int uartID)
{
  u32int uID = uartID-1;
  // init function: setup device, reset register values to defaults!
  uart[uID] = (struct Uart*)mallocBytes(sizeof(struct Uart));
  if (uart[uID] == 0)
  {
    serial_ERROR("Failed to allocate uart.");
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
    serial_ERROR("UART: loadUart invalid access size - byte");
  }

  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int uID = getUartNumber(phyAddr);
  if (uID == 0)
  {
    serial_ERROR("loadUart: invalid UART id.");
  }
  uID--; // array starts at 0
  u32int value = 0;
  u32int regOffs = phyAddr - getUartBaseAddr(uID+1);

  switch (regOffs)
  {
    case UART_MDR1_REG:
      value = uart[uID]->mdr1;
      break;
    case UART_MDR2_REG:
    case UART_UASR_REG:
    case UART_SCR_REG:
    case UART_SSR_REG:
    case UART_MVR_REG:
    case UART_SYSC_REG:
    case UART_SYSS_REG:
    case UART_WER_REG:
      dumpGuestContext(getGuestContext());
      serial_putstring("loadUart reg ");
      serial_putint_nozeros(regOffs);
      serial_newline();
      serial_ERROR("UART: load from unimplemented register.");
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("loadUart reg ");
      serial_putint_nozeros(regOffs);
      serial_newline();
      serial_ERROR("UART: load from undefined register.");
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
    serial_ERROR("UART: storeUart invalid access size - byte");
  }

  //We care about the real pAddr of the entry, not its vAddr
  GCONTXT* gc = getGuestContext();
  descriptor* ptd = gc->virtAddrEnabled ? gc->PT_shadow : gc->PT_physical;
  u32int phyAddr = getPhysicalAddress(ptd, address);
  u32int uID = getUartNumber(phyAddr);
  if (uID == 0)
  {
    serial_ERROR("storeUart: invalid UART id.");
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
      serial_ERROR(" storing to R/O register (SYSS_REG)");
      break;
    case UART_UASR_REG:
      serial_putstring(dev->deviceName);
      serial_ERROR(" storing to R/O register (autobaud status)");
      break;
    case UART_SSR_REG:
      serial_putstring(dev->deviceName);
      serial_ERROR(" storing to R/O register (SSR)");
      break;
    case UART_MVR_REG:
      serial_putstring(dev->deviceName);
      serial_ERROR(" storing to R/O register (MVR)");
      break;
    case UART_MDR2_REG:
    case UART_WER_REG:
      dumpGuestContext(getGuestContext());
      serial_putstring("storeUart reg ");
      serial_putint_nozeros(regOffs);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      serial_ERROR("UART: store to unimplemented register.");
    default:
      dumpGuestContext(getGuestContext());
      serial_putstring("storeUart reg ");
      serial_putint_nozeros(regOffs);
      serial_putstring(" value ");
      serial_putint(value);
      serial_newline();
      serial_ERROR("UART: store to undefined register.");
  } // switch ends

}

inline u32int getUartNumber(u32int phyAddr)
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

inline u32int getUartBaseAddr(u32int id)
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
      serial_ERROR("getUartBaseAddr: invalid base id.");
  }
  return -1;
}
