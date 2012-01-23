#ifndef __GUEST_BOOT__TEST_H__
#define __GUEST_BOOT__TEST_H__

#include "common/types.h"
#include "guestManager/guestContext.h"

#define TEST_IMAGE_HEADER_NAME_LENGTH  64

void bootTest(GCONTXT *context, u32int imageAddress)  __attribute__((noreturn));

struct testImageHeader getTestImageHeader(ulong uImageAddr);

struct testImageHeader
{
  u32int  ih_size;  /* Image Data Size    */
  u32int  ih_load;  /* Data   Load  Address    */
  u32int  ih_ep;    /* Entry Point Address    */
  u32int  ih_rsv;   /* Reserved */
  u8int   ih_name[TEST_IMAGE_HEADER_NAME_LENGTH];  /* Image Name    */
};

#endif  /* __GUEST_BOOT__TEST_H__ */
