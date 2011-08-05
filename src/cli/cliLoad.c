#include "cli/cliLoad.h"

#include "common/debug.h"


CLI_COMMAND_HANDLER(cliLoadBinary)
{
  if (argc == 1)
  {
    // offset
  }
  else
  {
    /*
     * Print usage
     */
    printf("Usage: loadBinary offset\r\n");
  }
}

CLI_COMMAND_HANDLER(cliLoadImage)
{
  if (argc == 0)
  {
    // nothing here
  }
  else
  {
    printf("Usage: loadImage\r\n");
  }
}
