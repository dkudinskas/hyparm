#include <stdio.h>

#include "cli/cli.h"
#include "drivers/beagle/beUart.h"

void run(const char *buffer);

CLI_COMMAND_HANDLER(cliLoadBinary) {}
CLI_COMMAND_HANDLER(cliLoadImage) {}

int main(int argc, char **argv)
{
  if (argc == 2)
  {
    printf("Input: [%s]\n", argv[1]);
    run(argv[1]);
    return 0;
  }
  printf("Usage: ./cli 'test string'\n");
  return 1;
}
