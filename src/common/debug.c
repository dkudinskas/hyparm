#include "cpuArch/armv7.h"
#include "cpuArch/constants.h"

#include "common/debug.h"
#include "common/stddef.h"
#include "common/string.h"

#include "drivers/beagle/beUart.h"

#ifdef CONFIG_MMC
#include "io/fs/fat.h"
#endif


#define TERMINAL_WIDTH  80


extern u32int abtStack;
extern u32int abtStackEnd;

extern u32int fiqStack;
extern u32int fiqStackEnd;

extern u32int irqStack;
extern u32int irqStackEnd;

extern u32int svcStack;
extern u32int svcStackEnd;

extern u32int undStack;
extern u32int undStackEnd;


#ifdef CONFIG_MMC
extern fatfs mainFilesystem;
extern file * debugStream;
#endif

#ifdef CONFIG_EMERGENCY_EXCEPTION_VECTOR
extern void setEmergencyExceptionVector(void);
#endif


const char *const ERROR_NO_SUCH_REGISTER = "no such register";
const char *const ERROR_NOT_IMPLEMENTED = "not implemented";


void dumpStackFromParameters(u32int snapshotOrigin, u32int psr, u32int *stack)
  __attribute__((externally_visible));


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

void dieNow(const char *file, const char *line, const char *caller, const char *msg)
{
#ifdef CONFIG_EMERGENCY_EXCEPTION_VECTOR
  setEmergencyExceptionVector();
#endif

#ifdef CONFIG_MMC
  fclose(&mainFilesystem, debugStream);
#endif

  banner("ERROR");
  printf("%s:%s: in %s:" EOL, file, line, caller);
  printf("%s" EOL, msg);

  const GCONTXT *context = getGuestContext();
  if (context != NULL)
  {
    dumpGuestContext(context);
  }
#ifdef CONFIG_DUMP_STACK
  dumpStack();
#endif
  banner("HALT");

  infiniteIdleLoop();
}

void dumpStack()
{
  __asm__ __volatile__("MOV R0, LR; MRS R1, CPSR; MOV R2, SP; B dumpStackFromParameters");
}

void dumpStackFromParameters(u32int snapshotOrigin, u32int psr, u32int *stack)
{
  const char *modeString;
  u32int *stackBegin, *stackEnd;

  switch (psr & PSR_MODE)
  {
    case PSR_SYS_MODE:
      modeString = "SYS";
      stackBegin = NULL;
      stackEnd = NULL;
      break;
    case PSR_FIQ_MODE:
      modeString = "FIQ";
      stackBegin = &fiqStack;
      stackEnd = &fiqStackEnd;
      break;
    case PSR_IRQ_MODE:
      modeString = "IRQ";
      stackBegin = &irqStack;
      stackEnd = &irqStackEnd;
      break;
    case PSR_SVC_MODE:
      modeString = "SVC";
      stackBegin = &svcStack;
      stackEnd = &svcStackEnd;
      break;
    case PSR_ABT_MODE:
      modeString = "ABT";
      stackBegin = &abtStack;
      stackEnd = &abtStackEnd;
      break;
    case PSR_UND_MODE:
      modeString = "UND";
      stackBegin = &undStack;
      stackEnd = &undStackEnd;
      break;
    default:
      modeString = "???";
      stackBegin = NULL;
      stackEnd = NULL;
      return;
  }

  printf("CPSR = %#.8x (mode: %s); SP = %p width PC = %#.8x" EOL, psr, modeString, stack, snapshotOrigin);

  if (stackBegin == NULL)
  {
    printf("Error: there is no stack associated with this mode!" EOL);
  }
  else
  {
    printf("Stack top: %p; limit: %p" EOL, stackBegin, stackEnd);
    if (stack > stackBegin)
    {
      printf("Error: stack for this mode is corrupt" EOL);
    }
    else
    {
      if (stack < stackEnd)
      {
        printf("Warning: stack pointer exceeds limit!" EOL);
      }
      else if (stack == stackEnd)
      {
        printf("Warning: stack pointer hit limit!" EOL);
      }

      while (stack < stackBegin)
      {
        stack++;
        printf("%p: %#.8x" EOL, stack, *stack);
      }
    }
  }
}

/*
 * WARNING: this function or any functions called from here must *NEVER* call the memory allocator
 * because it may be used to debug the allocator itself.
 */
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
