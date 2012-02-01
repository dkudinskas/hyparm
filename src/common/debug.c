#include "cpuArch/armv7.h"

#include "common/debug.h"
#include "common/string.h"

#include "drivers/beagle/beUart.h"

#ifdef CONFIG_MMC
#include "io/fs/fat.h"
#endif


#define TERMINAL_WIDTH  80


#ifdef CONFIG_MMC
extern fatfs mainFilesystem;
extern file * debugStream;
#endif

#ifdef CONFIG_EMERGENCY_EXCEPTION_VECTOR
extern void setEmergencyExceptionVector(void);
#endif


static void banner(const char *msg)
{
  u32int msgLength, paddingLength, i;
  char padding[TERMINAL_WIDTH >> 1];
  /*
   * Determine length of message and truncate if it exceeds the available space.
   */
  msgLength = strlen(msg);
  if (msgLength > (TERMINAL_WIDTH - 4))
  {
    msgLength = TERMINAL_WIDTH - 4;
  }
  paddingLength = (TERMINAL_WIDTH - 2 - msgLength) >> 1;
  /*
   * Creating padding string
   */
  for (i = 0; i < paddingLength; ++i)
  {
    padding[i] = '=';
  }
  padding[paddingLength] = 0;
  /*
   * Print it all at once
   */
  printf(EOL EOL "%s[%s]%s%s" EOL EOL, padding, msg, ((msgLength & 1) ? "" : "="), padding);
}

void DIE_NOW(GCONTXT *context, const char *msg)
{
#ifdef CONFIG_EMERGENCY_EXCEPTION_VECTOR
  setEmergencyExceptionVector();
#endif

#ifdef CONFIG_MMC
  fclose(&mainFilesystem, debugStream);
#endif

  banner("ERROR");
  printf("%s" EOL, msg);
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


#ifdef CONFIG_MMC

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

#endif /* CONFIG_MMC */
