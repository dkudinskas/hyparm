#include "common/debug.h"
#include "common/byteOrder.h"
#include "common/linker.h"
#include "common/string.h"

#include "guestBoot/test.h"
#include "guestBoot/loader.h"

#include "memoryManager/memoryConstants.h"


struct testImageHeader getTestImageHeader(ulong uImageAddr)
{
  u32int nmIndex;
  struct testImageHeader *hdrPtr = (struct testImageHeader *) uImageAddr;
  struct testImageHeader imgHdr;

  imgHdr.ih_size = ___swap32(hdrPtr->ih_size);    /* Image Data Size */
  imgHdr.ih_load = ___swap32(hdrPtr->ih_load);    /* Data Load Address */
  imgHdr.ih_ep   = ___swap32(hdrPtr->ih_ep);      /* Entry Point Address */
  for (nmIndex = 0; nmIndex < TEST_IMAGE_HEADER_NAME_LENGTH; nmIndex++)
  {
    imgHdr.ih_name[nmIndex] = hdrPtr->ih_name[nmIndex]; /* Image Name */
  }
  return imgHdr;
}

void bootTest(GCONTXT *context, u32int imageAddress)
{
  DEBUG(STARTUP, "bootTest: imageAddress = %#.8x" EOL, imageAddress);

  u32int currentAddress = imageAddress + sizeof(struct testImageHeader);

  struct testImageHeader imageHeader = getTestImageHeader(imageAddress);

  /*
   * LSB of entrypoint indicates if Thumb flag should be set
   */
  u32int loadAddress = imageHeader.ih_load;
  u32int entryPoint = imageHeader.ih_ep;
  u32int entryPointAddress = imageHeader.ih_ep & ~1;
  u32int sizeInBytes = imageHeader.ih_size;

  if (entryPointAddress > loadAddress + sizeInBytes || entryPointAddress < loadAddress)
  {
    printf("bootTest: entrypoint: %#.8x" EOL, entryPointAddress);
    DIE_NOW(context, "Wrong entrypoint");
  }
  if (currentAddress != loadAddress)
  {
    DEBUG(STARTUP, "bootTest: relocating test from %#.8x to %#.8x" EOL, currentAddress,
        loadAddress);
    memmove((void *)loadAddress, (const void *)currentAddress, sizeInBytes);
  }

  /*
   * Fill guest memory with fixed pattern to ensure no data is left from previous runs. Even after
   * a 'hard' reset, the power to the SDRAM is never cut so its contents remain.
   */
  const u32int endAddress = loadAddress + sizeInBytes;
  const u32int pattern = 0x88442211;
  DEBUG(STARTUP, "Filling guest memory with pattern %#.8x...", pattern);
  memset((void *)MEMORY_START_ADDR, pattern, loadAddress - MEMORY_START_ADDR);
  memset((void *)endAddress, pattern, HYPERVISOR_BEGIN_ADDRESS - endAddress);
  DEBUG(STARTUP, " done" EOL);

  bootGuest(context, GUEST_OS_TEST, entryPoint);
}
