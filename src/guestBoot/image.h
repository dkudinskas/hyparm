#ifndef __GUEST_BOOT__IMAGE_H__
#define __GUEST_BOOT__IMAGE_H__ 1

#include "common/types.h"


/*
 * Operating System Codes
 */
#define IH_OS_INVALID    0  /* Invalid OS  */
#define IH_OS_OPENBSD    1  /* OpenBSD  */
#define IH_OS_NETBSD    2  /* NetBSD  */
#define IH_OS_FREEBSD    3  /* FreeBSD  */
#define IH_OS_4_4BSD    4  /* 4.4BSD  */
#define IH_OS_LINUX    5  /* Linux  */
#define IH_OS_SVR4    6  /* SVR4    */
#define IH_OS_ESIX    7  /* Esix    */
#define IH_OS_SOLARIS    8  /* Solaris  */
#define IH_OS_IRIX    9  /* Irix    */
#define IH_OS_SCO    10  /* SCO    */
#define IH_OS_DELL    11  /* Dell    */
#define IH_OS_NCR    12  /* NCR    */
#define IH_OS_LYNXOS    13  /* LynxOS  */
#define IH_OS_VXWORKS    14  /* VxWorks  */
#define IH_OS_PSOS    15  /* pSOS    */
#define IH_OS_QNX    16  /* QNX    */
#define IH_OS_U_BOOT    17  /* Firmware  */
#define IH_OS_RTEMS    18  /* RTEMS  */
#define IH_OS_ARTOS    19  /* ARTOS  */
#define IH_OS_UNITY    20  /* Unity OS  */
#define IH_OS_INTEGRITY    21  /* INTEGRITY  */

/*
 * CPU Architecture Codes (supported by Linux)
 */
#define IH_ARCH_INVALID    0  /* Invalid CPU  */
#define IH_ARCH_ALPHA    1  /* Alpha  */
#define IH_ARCH_ARM    2  /* ARM    */
#define IH_ARCH_I386    3  /* Intel x86  */
#define IH_ARCH_IA64    4  /* IA64    */
#define IH_ARCH_MIPS    5  /* MIPS    */
#define IH_ARCH_MIPS64    6  /* MIPS   64 Bit */
#define IH_ARCH_PPC    7  /* PowerPC  */
#define IH_ARCH_S390    8  /* IBM S390  */
#define IH_ARCH_SH    9  /* SuperH  */
#define IH_ARCH_SPARC    10  /* Sparc  */
#define IH_ARCH_SPARC64    11  /* Sparc 64 Bit */
#define IH_ARCH_M68K    12  /* M68K    */
#define IH_ARCH_NIOS    13  /* Nios-32  */
#define IH_ARCH_MICROBLAZE  14  /* MicroBlaze   */
#define IH_ARCH_NIOS2    15  /* Nios-II  */
#define IH_ARCH_BLACKFIN  16  /* Blackfin  */
#define IH_ARCH_AVR32    17  /* AVR32  */
#define IH_ARCH_ST200          18  /* STMicroelectronics ST200  */

#define IH_TYPE_INVALID    0  /* Invalid Image    */
#define IH_TYPE_STANDALONE  1  /* Standalone Program    */
#define IH_TYPE_KERNEL    2  /* OS Kernel Image    */
#define IH_TYPE_RAMDISK    3  /* RAMDisk Image    */
#define IH_TYPE_MULTI    4  /* Multi-File Image    */
#define IH_TYPE_FIRMWARE  5  /* Firmware Image    */
#define IH_TYPE_SCRIPT    6  /* Script file      */
#define IH_TYPE_FILESYSTEM  7  /* Filesystem Image (any type)  */
#define IH_TYPE_FLATDT    8  /* Binary Flat Device Tree Blob  */
#define IH_TYPE_KWBIMAGE  9  /* Kirkwood Boot Image    */

/*
 * Compression Types
 */
#define IH_COMP_NONE    0  /*  No   Compression Used  */
#define IH_COMP_GZIP    1  /* gzip   Compression Used  */
#define IH_COMP_BZIP2    2  /* bzip2 Compression Used  */
#define IH_COMP_LZMA    3  /* lzma  Compression Used  */

#define IH_MAGIC  0x27051956  /* Image Magic Number    */
#define IH_NMLEN    32  /* Image Name Length    */

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
  u32int  ih_magic;  /* Image Header Magic Number  */
  u32int  ih_hcrc;  /* Image Header CRC Checksum  */
  u32int  ih_time;  /* Image Creation Timestamp  */
  u32int  ih_size;  /* Image Data Size    */
  u32int  ih_load;  /* Data   Load  Address    */
  u32int  ih_ep;    /* Entry Point Address    */
  u32int  ih_dcrc;  /* Image Data CRC Checksum  */
  u8int    ih_os;    /* Operating System    */
  u8int    ih_arch;  /* CPU architecture    */
  u8int    ih_type;  /* Image Type      */
  u8int    ih_comp;  /* Compression Type    */
  u8int    ih_name[IH_NMLEN];  /* Image Name    */
} image_header_t;

typedef struct image_info {
  ulong    start, end;    /* start/end of blob */
  ulong    image_start, image_len; /* start of image within blob, len of image */
  ulong    load;      /* load addr for the image */
  u8int    comp, type, os;    /* compression, type of image, os type */
} image_info_t;


image_header_t getImageHeader(ulong uImageAddr);
void dumpHdrInfo(image_header_t * imgHdr);

#endif  /* __IMAGE_H__ */
