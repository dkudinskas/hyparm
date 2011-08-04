#include "cli.h"

#include "common/ctype.h"

#define CLI_BUFFER_SIZE  64
#define CLI_MAX_TOKENS   10


void run(const char *command);

void enterCliLoop()
{
  char buffer[CLI_BUFFER_SIZE];
  char *const bufferEnd = buffer + (CLI_BUFFER_SIZE - 1);
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
        case '\b':
        case '\r':
        case '\n':
          break;
        default:
          serialPutc('\a');
          continue;
        }
      }
      serialPutc(*bufferPtr);
      ++bufferPtr;
    }
    if (bufferPtr == bufferEnd)
    {
      serialPuts("\r\nError: line too long\r\n");
      continue;
    }
    *bufferPtr = 0;
    serialPuts("\r\nBuffer is [");
    serialPuts(buffer);
    serialPuts("]\r\n");
  }
}

void run(const char *buffer)
{
  /*
   * Parse the string in the buffer into an array of tokens.
   */
  char token[CLI_BUFFER_SIZE];
  char *tokens[CLI_MAX_TOKENS];
  char *readPtr = buffer;
  char *writePtr = token;
  int tokenCount = 0;
  short quoted = 0;
  short escape = 0;
  while (*readPtr)
  {
    if (escape)
    {
      escape = 0;
      switch (*readPtr)
      {
      case '\'':
      case '\\':
        *writePtr++ = *readPtr;
        break;
      default:
        serialPuts("Error: invalid escape sequence '\\");
        serialPutc(*readPtr);
        serialPuts("'\r\n");
        return;
      }
    }
    else
    {
      switch (*readPtr)
      {
      case '\\':
        escape = 1;
        continue;
      case '\'':
        quoted = !quoted;
        continue;
      case ASCII_SPACE:
        if (!quoted)
        {
          /*
           * Outside single quotes, space acts as a token separator.
           */
	  int length = writePtr - token + 1;
	  char *thisToken = mallocBytes(length);
	  strncpy(thisToken, token, length - 1);
	  thisToken[length - 1] = 0;
	  tokens[tokenCount] = thisToken;
	  writePtr = token;
	  tokenCount++;
          continue;
	  // TODO: multiple spaces cause mult tokens
        }
      default:
        *writePtr++ = *readPtr;
        break;
      }
    }
    ++readPtr;
  }
}
