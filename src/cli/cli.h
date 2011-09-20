#ifndef __CLI__CLI_H__
#define __CLI__CLI_H__


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
#define CLI_MAX_TOKENS    4

/*
 * Command handler function pointer type.
 *
 * Each command handler must be of the following form:
 * void handler(int argc, char **argv);
 */
typedef void (*cliCommandHandler)(int argc, char argv[][CLI_BUFFER_SIZE]);


#define CLI_COMMAND_HANDLER(name)  void name(int argc, char argv[][CLI_BUFFER_SIZE])


void enterCliLoop(void)
#ifndef TEST_CLI
  __attribute__((noreturn))
#endif
  ;

#endif
