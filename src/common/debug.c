#include "cpuArch/cpu.h"

#include "common/debug.h"
#include "common/stringFunctions.h"

#include "drivers/beagle/beUart.h"

extern GCONTXT * getGuestContext(void); //from main.c

#ifdef DIE_NOW_SCANNER_COUNTER
  extern u32int scannerReqCounter; //from scanner.c
#endif

void banner(char* msg)
{
  DEBUG_NEWLINE();
  int messageLength, paddingLength, i;
  char* pos = msg;
  while(*pos != '\0')
  {
    pos++;
  }
  messageLength = pos - msg;
  paddingLength = (76 - messageLength)/2;
  
  for(i = 0; i < paddingLength; i++)
  {
    DEBUG_CHAR('=');
  }
  DEBUG_STRING("[ ");
  DEBUG_STRING(msg);
  DEBUG_STRING("] ");
  for(i = 0; i < paddingLength; i++)
  {
    DEBUG_CHAR('=');
  }
  if(messageLength % 2 == 1)
  {
    DEBUG_CHAR('=');
  }
  DEBUG_NEWLINE();
  DEBUG_NEWLINE();
}

void DIE_NOW(GCONTXT* context, char* msg)
{
  banner("ERROR");
  DEBUG_STRING(msg);
  DEBUG_NEWLINE();
#ifdef DIE_NOW_SCANNER_COUNTER
  DEBUG_STRING("Number of scanned blocks: ");
  DEBUG_INT(scannerReqCounter);
  DEBUG_NEWLINE();
#endif
  if (context == 0)
  {
    context = getGuestContext();
  }
  if (context != 0)
  {
    dumpGuestContext(context);
  }
  banner("HALT");
  
  infiniteIdleLoop();
}


void __attribute__((noinline)) DEBUG_CHAR(char c)
{
  while ((beLoadUart(UART_LSR_REG, 3) & UART_LSR_TX_FIFO_E) == 0)
  {
    // do nothing
  }
  beStoreUart(UART_THR_REG, (u32int)c, 3);
  
}

void DEBUG_STRING(char * c)
{
  int index = 0;

  while (printableChar(c[index]))
  {
    DEBUG_CHAR(c[index]);
    index++;
  }
  return;
}

void DEBUG_NEWLINE()
{
  DEBUG_CHAR('\r');
  DEBUG_CHAR('\n');
  return;
}

void DEBUG_INT(u32int nr)
{
  int length = 8;
  int lengthInBits = 32;
  int byte = 0;
  int i = 0;

  for (i = 0; i < length; i++)
  {
    byte = nr >> ( lengthInBits - ((i+1) * 4) );
    byte = byte & 0xF;
    DEBUG_BYTE(byte);
  }
}

//Don't output leading zeros
void DEBUG_INT_NOZEROS(u32int nr)
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
      DEBUG_BYTE(byte);
    }
  }
}

void DEBUG_BYTE(u8int nr)
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
  DEBUG_CHAR(byte);
}


u32int printf(const char *fmt, ...)
{
  va_list args;
  u32int i;
  char printbuffer[256];
  va_start(args, fmt);

  i = vsprintf(printbuffer, fmt, args);
  va_end(args);

  /* Print the string */
  DEBUG_STRING(printbuffer);
  return i;
}


u32int vsprintf(char *buf, const char *fmt, va_list args)
{
  char *str;

  str = buf;

  for (; *fmt ; ++fmt)
  {
    if ((*fmt != '%') && (*fmt != '\n'))
    {
      *str++ = *fmt;
      continue;
    }

    if (*fmt == '%')
    {
      // skip %
      ++fmt;

      u32int zeroPadding = 1;
      switch (*fmt)
      {
        case '0':
        {
          // zero padding!
          // skip %
          ++fmt;
          zeroPadding = *fmt++;
          if ((zeroPadding < 0x31) || (zeroPadding > 0x38))
          {
            DIE_NOW(0, "invalid padding bits.");
          }
          zeroPadding -= 0x30;
        }
        case 'x':
        {
          u32int number = va_arg(args, int);

          int length = 8;
          int lengthInBits = 32;
          int byte = 0;
          int i = 0;
          bool keepZeros = FALSE;
        
          for (i = 0; i < length; i++)
          {
            byte = number >> ( lengthInBits - ((i+1) * 4) );
            byte = byte & 0xF;

            if (byte != 0)
            {
              keepZeros = TRUE;
            }
        
            if ( keepZeros || i >= (7-(zeroPadding-1)) )
            {
              if ( (byte >= 0) && (byte <= 9) )
              {
                byte = byte + 0x30;
              }
              else
              {
                switch(byte)
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
                } // switch ends
              } // else ends
              *str++ = byte;
            }
          } // for ends - whole number is now done
          break;
        }
        case 'c':
        {
          char character = va_arg(args, char);
          *str++ = character;
          break;
        }
        default:
        {
          DEBUG_STRING("option after %: ");
          DEBUG_CHAR(*fmt);
          DEBUG_NEWLINE();
          DIE_NOW(0, "Unknown option after %");
        }
      } // switch ends
    } // if % character found

    if (*fmt == '\n')
    {
      *str++ = '\r';
      *str++ = '\n';
      break;
    }

  } // for ends
  *str = '\0';
  return stringlen(str);
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
