#include "cpuArch/cpu.h"

#include "vm/omap35xx/serial.h"

#include "drivers/beagle/beUart.h"

void __attribute__((noinline)) serial_putchar(char c)
{
  while ((beLoadUart(UART_LSR_REG, 3) & UART_LSR_TX_FIFO_E) == 0)
  {
    // do nothing
  }
  beStoreUart(UART_THR_REG, (u32int)c, 3);
  
}

void serial_putstring(char * c)
{
  int index = 0;

  while (printableChar(c[index]))
  {
    serial_putchar(c[index]);
    index++;
  }
  return;
}

void serial_newline()
{
  serial_putchar('\r');
  serial_putchar('\n');
  return;
}

void serial_putlong(ulong nr)
{
  int length = 8;
  int lengthInBits = 32;
  int byte = 0;
  int i = 0;

  for (i = 0; i < length; i++)
  {
    byte = nr >> ( lengthInBits - ((i+1) * 4) );
    byte = byte & 0xF;
    serial_putbyte(byte);
  }
}

void serial_putint(u32int nr)
{
  int length = 8;
  int lengthInBits = 32;
  int byte = 0;
  int i = 0;

  for (i = 0; i < length; i++)
  {
    byte = nr >> ( lengthInBits - ((i+1) * 4) );
    byte = byte & 0xF;
    serial_putbyte(byte);
  }
}

//Don't output leading zeros
void serial_putint_nozeros(u32int nr)
{
  int length = 8;
  int lengthInBits = 32;
  int byte = 0;
  int i = 0;
  bool keepZeros = FALSE;

  for (i = 0; i < length; i++)
  {
    byte = nr >> ( lengthInBits - ((i+1) * 4) );
    byte = byte & 0xF;
    if(0 != byte)
    {
      keepZeros = TRUE;
    }

    if(keepZeros || i==7)
    {
      serial_putbyte(byte);
    }
  }
}

void serial_putbyte(u8int nr)
{
  u8int tmpNr = nr & 0xF;
  u8int byte = 0;
  if ( (tmpNr >= 0) && (tmpNr <= 9) )
  {
    byte = tmpNr + 0x30;
  }
  else
  {
    switch(tmpNr)
    {
      case 0xa:
        byte = 0x61;
        break;
      case 0xb:
        byte = 0x62;
        break;
      case 0xc:
        byte = 0x63;
        break;
      case 0xd:
        byte = 0x64;
        break;
      case 0xe:
        byte = 0x65;
        break;
      case 0xf:
        byte = 0x66;
        break;
      default:
        return;
    } // switch ends
  } // else ends
  serial_putchar(byte);
}

int printableChar(char c)
{
  if ( (c == LINE_FEED) || (c == CARRIAGE_RETURN) )
  {
    return 1;
  }
  else if ( (c >= PRINTABLE_START) && (c <= PRINTABLE_END) )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
