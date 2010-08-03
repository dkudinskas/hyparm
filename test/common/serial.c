#include "serial.h"
#include <stdio.h>
#include "types.h"

/* Need to mimic what the serial thingy does,
 * so pulling the header from the main build and implementing what we need for testing here
*/

void serial_putchar(char c)
{
  printf("%c",c);
}

void serial_putstring(char * c)
{
  printf("%s",c);
}

void serial_newline(void)
{
  printf("\n");
}

void serial_putlong(ulong nr)
{
  printf("%ld", nr);
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
  printf("%c", byte);
}

int printableChar(char c)
{
  printf("\n\nSerial.c\nprintableChar method not implemented!\nIn infinite loop\n\n");

  while(TRUE)
  {
    /* infinite loop */
  }

  return 0; //make compiler happy
}

void serial_ERROR(char * msg)
{
  serial_putstring(msg);
  printf("\n");
  while(TRUE)
  {
    // HALT EXECUTION HERE
  }
}
