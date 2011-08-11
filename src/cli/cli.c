#include "cli/cli.h"
#include "cli/cliLoad.h"

#include "common/ctype.h"
#include "common/debug.h"
#include "common/stringFunctions.h"

#include "drivers/beagle/beUart.h"


/*
 * Command
 */
struct cliCommand
{
  const char *const command;
  cliCommandHandler const handler;
};

/*
 * Number of commands in the command table.
 */
#define CLI_NUM_COMMANDS  2

/*
 * Command table (in alphabetical order of command names).
 */
static struct cliCommand commandTable[CLI_NUM_COMMANDS] =
{
  { "loadBinary", cliLoadBinary },
  { "loadImage", cliLoadImage }
};

/*
 * Structure to hold the state of the CLI parser.
 */
struct parseState
{
  unsigned quoted       : 8;
  unsigned escape       : 8;
  unsigned loop         : 8;
  unsigned tokenCount   : 8;
};


static s32int findCommand(const char *command);

#ifndef TEST
static
#endif
  void run(const char *command);


void enterCliLoop()
{
  char buffer[CLI_BUFFER_SIZE];
  char *const bufferEnd = buffer + (CLI_BUFFER_SIZE - 1);
  int ignore_n = 1;
  while (1)
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
              *bufferPtr = serialGetc();
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
    run(buffer);
  }
}

static s32int findCommand(const char *command)
{
  s32int min = 0;
  s32int max = CLI_NUM_COMMANDS - 1;
  s32int cmp, mid;
  do
  {
    mid = (min + max) >> 1;
    cmp = strcmp(command, commandTable[mid].command);
    if (cmp > 0)
    {
      min = mid + 1;
    }
    else
    {
      max = mid - 1;
    }
  }
  while (cmp && min <= max);
  return cmp ? -1 : mid;
}

#ifndef TEST
static
#endif
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
#ifdef TEST_CLI
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
#endif
    s32int commandIndex = findCommand(tokens[0]);
    if (commandIndex < 0)
    {
      printf("Error: command '%s' not found\r\n", tokens[0]);
      return;
    }
#ifndef TEST_CLI
    (*(commandTable[commandIndex].handler))(state.tokenCount - 1, &tokens[1]);
#endif
  }
}
