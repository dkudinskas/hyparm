#include "cpuArch/cpu.h"

#include "common/debug.h"

#include "hardware/serial.h"


extern GCONTXT * getGuestContext(void); //from main.c


void banner(char* msg)
{
  serial_newline();
  int messageLength, paddingLength, i;
  char* pos = msg;
  while(*pos != '\0') pos++;
  messageLength = pos - msg;
  paddingLength = (76 - messageLength)/2;
  
  for(i = 0; i < paddingLength; i++) serial_putchar('=');
  serial_putstring("[ ");
  serial_putstring(msg);
  serial_putstring("] ");
  for(i = 0; i < paddingLength; i++) serial_putchar('=');
  if(messageLength % 2 == 1)
    serial_putchar('=');
  serial_newline();
  serial_newline();
}

void DIE_NOW(GCONTXT* context, char* msg)
{
  banner("ERROR");
  serial_putstring(msg);
  serial_newline();
#ifdef DIE_NOW_SCANNER_COUNTER
  serial_putstring("Number of scanned blocks: ");
  serial_putint(scannerReqCounter);
  serial_newline();
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

