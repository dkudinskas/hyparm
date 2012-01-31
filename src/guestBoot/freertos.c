#include "common/debug.h"

#include "guestBoot/freertos.h"
#include "guestBoot/loader.h"


void bootFreeRtos(GCONTXT *context, u32int loadAddress)
{
  DEBUG(STARTUP, "bootFreeRtos: loadAddress = %#.8x" EOL, loadAddress);

  struct tag *tagList = getTagListBaseAddress();
  setupStartTag(&tagList);
  setupRevisionTag(&tagList);
  setupMemoryTags(&tagList);
  setupEndTag(&tagList);

  bootGuest(context, loadAddress);
}
