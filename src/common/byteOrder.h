#ifndef __BYTEORDER_H__ 
#define __BYTEORDER_H__ 

#define ___swap32(x) \
  ((u32int)( \
    (((u32int)(x) & (u32int)0x000000ffUL) << 24) | \
    (((u32int)(x) & (u32int)0x0000ff00UL) <<  8) | \
    (((u32int)(x) & (u32int)0x00ff0000UL) >>  8) | \
    (((u32int)(x) & (u32int)0xff000000UL) >> 24) ))

#define __be32_to_cpu(x) ___swap32(x)

#endif

