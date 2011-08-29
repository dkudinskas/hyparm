#include "cli/cliLoad.h"

#include "common/ctype.h"
#include "common/debug.h"
#include "common/stdio.h"


/*
 * FIXME: all of this code assumes virtual memory is turned off!
 */

/*
 * Maximum chunk size is 10 kB
 */
#define MAX_CHUNK_SIZE                 10240

/*
 * FIXME dirty
 */
#define VALID_MEMORY_RANGE_BEGIN  0x80000000
#define VALID_MEMORY_RANGE_END    0x8c000000


CLI_COMMAND_HANDLER(cliLoadBinary)
{
  if (argc == 1)
  {
    s32int charsConsumed = 0;
    u32int controlChar;
    volatile u8int *offset;
    s32int result = sscanf(argv[0], "%10x", &offset);
    if (result == 1)
    {
      u32int size;
      /*
       * Make sure offset is in RAM and not part of hypervisor memory
       */
      if ((u32int)offset < VALID_MEMORY_RANGE_BEGIN || (u32int)offset >= VALID_MEMORY_RANGE_END)
      {
        printf("Error: target offset %p outside available memory" EOL, offset);
        return;
      }
      /*
       * Send offset back to host
       */
      putchar(ASCII_NUL);
      printf("%cHYPARM%.8x", ASCII_BEL, offset);
      putchar(ASCII_NUL);
      /*
       * Get data size from host
       */
      if ((controlChar = getchar()) != ASCII_SOH)
      {
        printf("%c\rError: invalid data received from client (%#.2x); expected SOH." EOL, ASCII_NAK, controlChar);
        return;
      }
      {
        char buffer[9];
        int i;
        for (i = 0; i < 9; ++i)
        {
          buffer[i] = getchar();
        }
        if (buffer[8] != ASCII_ETB)
        {
          printf("%c\rError: invalid data received from client (%#.2x); expected ETB." EOL, ASCII_NAK, (u32int)buffer[8]);
          return;
        }
        buffer[8] = 0;
        if (sscanf(buffer, "%8x%n", &size, &charsConsumed) != 1 && charsConsumed != 8)
        {
          printf("%c\rError: invalid data received from client; expected size in hex." EOL, ASCII_NAK);
          return;
        }
      }
      /*
       * Make sure size is not zero...
       */
      if (size == 0)
      {
        printf("%c\rError: size received from client is 0 bytes." EOL, ASCII_NAK);
        return;
      }
      /*
       * First check with entire size of available memory to prevent overflow
       */
      if (size > (VALID_MEMORY_RANGE_END - VALID_MEMORY_RANGE_BEGIN))
      {
        printf("%c\rError: size received from client (%#.8x) exceeds total available memory." EOL, ASCII_NAK, size);
        return;
      }
      /*
       * Now check subrange
       */
      if (((u32int)offset + size) >= VALID_MEMORY_RANGE_END)
      {
        printf("%c\rError: size received from client (%#.8x) exceeds available memory starting from offset %#.8x." EOL, ASCII_NAK, size);
        return;
      }
      /*
       * Things seem fine, send ACK and wait for data to come in.
       */
      putchar(ASCII_ACK);
      {
        u16int chunkSize;
        u16int received = 0;
        while (received < size)
        {
          chunkSize = size < MAX_CHUNK_SIZE ? size : MAX_CHUNK_SIZE;
          for (received = 0; received < chunkSize; ++received)
          {
            *offset++ = (u8int)getchar();
          }
          size -= chunkSize;
          if ((controlChar = getchar()) == ASCII_ETB)
          {
            putchar(ASCII_ACK);
          }
          else
          {
            printf("%c\rError: invalid data received from client (%#.2x); expected ETB (%#.2x)." EOL, ASCII_NAK, controlChar, ASCII_ETB);
            return;
          }
        }
      }
      if ((controlChar = getchar()) == ASCII_EM)
      {
        printf("%c\rTransfer completed succesfully." EOL);
      }
      else
      {
        printf("%c\rError: invalid data received from client (%#.2x); expected EM (%#.2x)." EOL, ASCII_NAK, controlChar, ASCII_EM);
      }
      return;
    }
  }
  /*
   * Print usage
   */
  printf("Usage: loadBinary offset" EOL);
}

CLI_COMMAND_HANDLER(cliLoadImage)
{
  if (argc == 0)
  {
    // nothing here
  }
  else
  {
    printf("Usage: loadImage" EOL);
  }
}
