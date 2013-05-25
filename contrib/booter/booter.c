#include "image.h"
#include "loader.h"


void *memmove(void *destination, const void *source, u32int count);
void setupTimer(void);
extern void callKernel(s32int, s32int, struct tag *tagList, u32int entryPoint) __attribute__((noreturn));

int _start()
{
  u32int loadAddress = 0x80300000;

  u32int currentAddress = loadAddress + sizeof(image_header_t);
  image_header_t imageHeader = getImageHeader(loadAddress);
  u32int targetAddress = imageHeader.ih_load;
  u32int entryPoint = imageHeader.ih_ep;
  u32int sizeInBytes = imageHeader.ih_size;
  if (currentAddress != targetAddress)
  {
    memmove((void *)targetAddress, (const void *)currentAddress, sizeInBytes);
  }

  struct tag *tagList = getTagListBaseAddress();
  setupStartTag(&tagList);
  setupRevisionTag(&tagList);
  setupMemoryTags(&tagList);
  setupEndTag(&tagList);

  setupTimer();
  callKernel(0, BOARD_MACHINE_ID, getTagListBaseAddress(), entryPoint);
}


void setupTimer()
{
  __asm__ __volatile__("MOV    R8, #0\n\t"
                      "MCR     p15, 0, R8, c9, c12, 5\n\t"  /* select performance counter 0 */
                      "MOV     R8, #1\n\t"
                      "MCR     p15, 0, R8, c9, c13, 1\n\t"  /* get it to count i-cache L1 misses */

                      "MOV     R8, #1\n\t"
                      "MCR     p15, 0, R8, c9, c12, 5\n\t"  /* select performance counter 1 */
                      "MOV     R8, #3\n\t"
                      "MCR     p15, 0, R8, c9, c13, 1\n\t"  /* get it to count d-cache L1 misses */

                      "MOV     R8, #2\n\t"
                      "MCR     p15, 0, R8, c9, c12, 5\n\t"  /* select performance counter 2 */
                      "MOV     R8, #0x44\n\t"
                      "MCR     p15, 0, R8, c9, c13, 1\n\t"  /* get it to count L2 misses */

                      "MOV     R8, #3\n\t"
                      "MCR     p15, 0, R8, c9, c12, 5\n\t"  /* select performance counter 3 */
                      "MOV     R8, #5\n\t"
                      "MCR     p15, 0, R8, c9, c13, 1\n\t"  /* get to count d-tlb misses */

                      "MOV     R8, #0\n\t"
                      "ORR     R8, R8, #1\n\t"              /* enable all counters */
                      "ORR     R8, R8, #2\n\t"              /* reset all performance counters to 0 */
                      "ORR     R8, R8, #4\n\t"              /* reset cycle counter to 0 */
                      "ORR     R8, R8, #8\n\t"              /* ENABLE the divider (64)*/
                      "ORR     R8, R8, #16\n\t"             /* not sure: enable export events? */
                      "MCR     p15, 0, R8, c9, c12, 0\n\t"  /* Write PMNC Register */
                      "MOVW    R8, #0x000f\n\t"
                      "MOVT    R8, #0x8000\n\t"
                      "MCR     p15, 0, R8, c9, c12, 1\n\t"  /* enable all counters */
                      "MOV     R7, #0\n\t"
                      "MCR     p15, 0, R7, c9, c12, 2\n\t"  /* enable all counters */
                      "MCR     p15, 0, R8, c9, c12, 3\n\t"  /* clear all overflow flags */
                      : :);

}


/**
 * memory move
 */
void *memmove(void *destination, const void *source, u32int count)
{
  const char *src;
  char *dst;

  if (destination <= source)
  {
    dst = (char *)destination;
    src = (const char *)source;
    while (count--)
    {
      *dst++ = *src++;
    }
  }
  else
  {
    dst = (char *)destination + count;
    src = (const char *)source + count;
    while (count--)
    {
      *--dst = *--src;
    }
  }
  return destination;
}
