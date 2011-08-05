#include "cli.h"

#include "common/ctype.h"
#include "common/stringFunctions.h"

#include "drivers/beagle/beUart.h"


/*
 * Size of the buffer in which command line input is stored.
 */
#define CLI_BUFFER_SIZE  64

/*
 * Maximum number of tokens the input is split into.
 * The maximum number of 'arguments' to any 'command' is (CLI_MAX_TOKENS - 1).
 *
 * CAUTION: for each token, a buffer of size CLI_BUFFER_SIZE is allocated on the stack.
 * Only increase this number if you are sure you will not cause a stack overflow!
 */
#define CLI_MAX_TOKENS   4


struct parseState
{
  unsigned quoted : 8;
  unsigned escape : 8;
  unsigned loop : 8;
  unsigned tokenCount : 8;
};


void run(const char *command);


void enterCliLoop()
{
  char buffer[CLI_BUFFER_SIZE];
  char *const bufferEnd = buffer + (CLI_BUFFER_SIZE - 1);
  int ignore_n = 1;
#ifdef TEST_CLI
  int loopCount = 0;
  for (loopCount = 0; loopCount < 10; ++loopCount)
#else
  while (1)
#endif
  {
    serialPuts("H> ");
    char *bufferPtr = buffer;
    while (bufferPtr < bufferEnd)
    {
      *bufferPtr = serialGetc();
      if (iscntrl(*bufferPtr))
      {
        switch (*bufferPtr)
        {
        case ASCII_ESC:
          *bufferPtr = serialGetc();
          if (*bufferPtr == '[')
          {
            *bufferPtr == serialGetc();
            continue;
          }
          break;
        case '\b':
          if (bufferPtr > buffer)
          {
            --bufferPtr;
            serialPuts("\b \b");
          }
          continue;
        case '\n':
          if (ignore_n)
          {
            ignore_n = 0;
            continue;
          }
        case '\r':
          break;
        default:
          continue;
        }
      }
      serialPutc(*bufferPtr);
      if (*bufferPtr == '\r')
      {
        serialPutc('\n');
        ignore_n = 1;
        break;
      }
      if (*bufferPtr == '\n')
      {
        break;
      }
      ++bufferPtr;
    }
    if (bufferPtr == bufferEnd)
    {
      serialPuts("\r\nError: line too long\r\n");
      continue;
    }
    *bufferPtr = 0;
    serialPuts("Buffer is [");
    serialPuts(buffer);
    serialPuts("]\r\n");
    run(buffer);
  }
}

void run(const char *buffer)
{
  /*
   * Parse the string in the buffer into an array of tokens.
   */
  char token[CLI_BUFFER_SIZE];
  char tokens[CLI_MAX_TOKENS][CLI_BUFFER_SIZE];
  const char *readPtr = buffer;
  char *writePtr = token;
  struct parseState state = { 0, 0, 1, 0 };
  while (state.loop)
  {
    if (state.escape)
    {
      state.escape = 0;
      switch (*readPtr)
      {
      case '\'':
      case '\\':
        *writePtr++ = *readPtr;
        break;
      default:
        serialPuts("Error: invalid escape sequence");
        if (isprint(*readPtr))
        {
          serialPuts(" '\\");
          serialPutc(*readPtr);
          serialPutc('\'');
        }
        serialPuts("\r\n");
        return;
      }
    }
    else
    {
      switch (*readPtr)
      {
      case '\\':
        state.escape = 1;
        continue;
      case '\'':
        state.quoted = !state.quoted;
        continue;
      case '\0':
      case ASCII_SPACE:
        if (!state.quoted)
        {
          /*
           * Outside single quotes, space acts as a token separator.
           * Only split when there is already some content in the token buffer;
           * this avoids creating multiple tokens for subsequent spaces.
           */
          if (writePtr != token)
          {
            *writePtr++ = '\0';
            stringcpy(tokens[state.tokenCount], token);
            writePtr = token;
            ++state.tokenCount;
          }
          state.loop = *readPtr;
          break;
        }
      default:
        if (isprint(*readPtr))
        {
          if (state.tokenCount < CLI_MAX_TOKENS)
          {
            *writePtr++ = *readPtr;
          }
          else
          {
            serialPuts("Error: maximum number of tokens exceeded\r\n");
            return;
          }
        }
        else
        {
          serialPuts("Error: invalid character\r\n");
          return;
        }
        break;
      }
    }
    ++readPtr;
  }
  if (state.tokenCount > 0)
  {
    int i;
    serialPuts("Command: '");
    serialPuts(tokens[0]);
    serialPuts("'\r\n");
    serialPuts("Arguments:\r\n");
    for (i = 1; i < state.tokenCount; ++i)
    {
      serialPutc('\'');
      serialPuts(tokens[i]);
      serialPuts("'\r\n");
    }
    serialPuts("Done\r\n");
  }
}