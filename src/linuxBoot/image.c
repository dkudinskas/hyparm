#include "image.h"
#include "common.h"

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


void dumpHdrInfo(image_header_t * imgHdr)
{
  serial_putstring("magic number: \n\r");
  serial_putint(imgHdr->ih_magic);
  serial_newline();

  serial_putstring("size: \n\r");
  serial_putint(imgHdr->ih_size);
  serial_newline();

  serial_putstring("data load addr: \n\r");
  serial_putint(imgHdr->ih_load);
  serial_newline();

  serial_putstring("entry point: \n\r");
  serial_putint(imgHdr->ih_ep);
  serial_newline();

  serial_putstring("OS: \n\r");
  serial_putbyte(imgHdr->ih_os);
  serial_newline();

  serial_putstring("arch: \n\r");
  serial_putbyte(imgHdr->ih_arch);
  serial_newline();

  serial_putstring("type: \n\r");
  serial_putbyte(imgHdr->ih_type);
  serial_newline();

  serial_putstring("compression: \n\r");
  serial_putbyte(imgHdr->ih_comp);
  serial_newline();

  serial_putstring("name: \n\r");
  serial_putstring((char*)&(imgHdr->ih_name));
  serial_newline();
  return;
}
