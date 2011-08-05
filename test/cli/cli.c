#include "drivers/beagle/beUart.h"

void run(const char *buffer);

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
