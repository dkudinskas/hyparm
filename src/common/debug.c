#include "cpuArch/cpu.h"

#include "common/debug.h"
#include "common/stringFunctions.h"

#include "drivers/beagle/beUart.h"

extern GCONTXT * getGuestContext(void); //from main.c

void banner(char* msg)
{
  printf("\r\n");
  printf("\r\n");
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
    printf("=");
  }
  printf("[");
  printf(msg);
  printf("]");
  for(i = 0; i < paddingLength; i++)
  {
    printf("=");
  }
  if(messageLength % 2 == 1)
  {
    printf("=");
  }
  printf("\r\n");
  printf("\r\n");
}

void DIE_NOW(GCONTXT* context, char* msg)
{
  banner("ERROR");
  printf(msg);
  printf("\r\n");
  if (context == 0)
  {
    context = getGuestContext();
  }
  if (context != 0)
  {
    dumpGuestContext(context);
  }
  banner("HALT\0");
  
  infiniteIdleLoop();
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
  serialPuts(printbuffer);
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
            DIE_NOW(0, "invalid padding bits.\0");
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
          serialPuts("option after %: ");
          serialPutc(*fmt);
          serialPuts("\r\n");
          DIE_NOW(0, "Unknown option after %\0");
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
