#include "image.h"


static inline u32int bs32(u32int number)
{
  u32int retVal = 0;
  retVal |= ((number & 0xFF000000) >> 24);
  retVal |= ((number & 0x00FF0000) >>  8);
  retVal |= ((number & 0x0000FF00) <<  8);
  retVal |= ((number & 0x000000FF) << 24);
  return retVal;
}

image_header_t getImageHeader(ulong uImageAddr)
{
  u32int nmIndex;
  image_header_t * hdrPtr = (image_header_t *)uImageAddr;
  image_header_t imgHdr;

  imgHdr.ih_magic = hdrPtr->ih_magic;        /* Image Header Magic Number */
  imgHdr.ih_hcrc = hdrPtr->ih_hcrc;          /* Image Header CRC Checksum  */
  imgHdr.ih_time = hdrPtr->ih_time;          /* Image Creation Timestamp  */
  imgHdr.ih_size = bs32(hdrPtr->ih_size);    /* Image Data Size */
  imgHdr.ih_load = bs32(hdrPtr->ih_load);    /* Data Load Address */
  imgHdr.ih_ep   = bs32(hdrPtr->ih_ep);      /* Entry Point Address */
  imgHdr.ih_dcrc = hdrPtr->ih_dcrc;          /* Image Data CRC Checksum */
  imgHdr.ih_os   = hdrPtr->ih_os;            /* Operating System */
  imgHdr.ih_arch = hdrPtr->ih_arch;          /* CPU architecture */
  imgHdr.ih_type = hdrPtr->ih_type;          /* Image Type */
  imgHdr.ih_comp = hdrPtr->ih_comp;          /* Compression Type */
  for (nmIndex = 0; nmIndex < IH_NMLEN; nmIndex++)
  {
    imgHdr.ih_name[nmIndex] = hdrPtr->ih_name[nmIndex]; /* Image Name */
  }
  return imgHdr;
}
