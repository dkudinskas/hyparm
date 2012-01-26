#include "cpuArch/cpu.h"

#include "common/debug.h"
#include "common/stringFunctions.h"

#include "drivers/beagle/beUart.h"

#include "io/fs/fat.h"


extern GCONTXT * getGuestContext(void); //from main.c

#ifdef CONFIG_MMC
extern fatfs mainFilesystem;
extern file * debugStream;
#endif

#ifdef DIE_NOW_SCANNER_COUNTER
  extern u32int scannerReqCounter; //from scanner.c
#endif

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
#ifdef DIE_NOW_SCANNER_COUNTER
  printf("Number of scanned blocks: %08x\r\n", scannerReqCounter);
#endif
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

#if CONFIG_MMC
u32int fprintf(const char *fmt, ...)
{
  va_list args;
  u32int i;
  char printbuffer[256];
  va_start(args, fmt);

  i = vsprintf(printbuffer, fmt, args);
  va_end(args);

  /* Print the string */
  fwrite(&mainFilesystem, debugStream, printbuffer, strlen(printbuffer));

  return i;
}
#endif
