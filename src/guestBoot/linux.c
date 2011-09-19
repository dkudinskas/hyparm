#include "common/debug.h"
#include "common/memFunctions.h"

#include "guestBoot/loader.h"
#include "guestBoot/linux.h"


// hardcoded initrd len?!
#define BOARD_INITRD_LEN     0x800000


void bootLinux(GCONTXT *context, image_header_t *imageHeader, u32int loadAddress, u32int initrdAddress)
{
  u32int currentAddress = loadAddress + sizeof(image_header_t);
  u32int targetAddress = imageHeader->ih_load;
  u32int entryPoint = imageHeader->ih_ep;
  u32int sizeInBytes = imageHeader->ih_size;

  DEBUG(STARTUP, "bootLinux: current address = %#.8x, target address = %#.8x, entry point = %#.8x"
      EOL, currentAddress, targetAddress, entryPoint);

  if (currentAddress != targetAddress)
  {
    DEBUG(STARTUP, "bootLinux: relocating kernel" EOL)
    memmove((void *)targetAddress, (const void *)currentAddress, sizeInBytes);
  }

  struct tag *tagList = getTagListBaseAddress();
  setupStartTag(&tagList);
  setupRevisionTag(&tagList);
  setupMemoryTags(&tagList);
  if (initrdAddress != 0)
  {
    setupInitrdTag(&tagList, initrdAddress, initrdAddress + BOARD_INITRD_LEN);
  }
  setupEndTag(&tagList);

  bootGuest(context, entryPoint);
}
