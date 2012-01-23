#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/byteOrder.h"

#include "guestBoot/test.h"
#include "guestBoot/loader.h"

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

  u32int loadAddress = imageHeader.ih_load;
  u32int entryPoint = imageHeader.ih_ep;
  u32int sizeInBytes = imageHeader.ih_size;

  if (entryPoint > loadAddress + sizeInBytes || entryPoint < loadAddress)
  {
    printf("bootTest: entrypoint: %#.8x" EOL, entryPoint);
    DIE_NOW(context, "Wrong entrypoint");
  }
  if (currentAddress != loadAddress)
  {
    DEBUG(STARTUP, "bootTest: relocating test from %#.8x to %#.8x" EOL, currentAddress,
        loadAddress);
    memmove((void *)loadAddress, (const void *)currentAddress, sizeInBytes);
  }

  bootGuest(context, GUEST_OS_TEST, entryPoint);
}
