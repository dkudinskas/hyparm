#ifndef __COMMON__LINKER_H__
#define __COMMON__LINKER_H__

#include "common/types.h"


/*
 * Macros to get information about the layout of the hypervisor in memory.
 *
 *
 * The object code generated by the compiler and the final output of the linker are in ELF format.
 * ELF is a very flexible format for executables and libraries.
 *
 * The compiler and linker will keep 'parts' of the hypervisor with a different purpose in
 * different places. In terms of the ELF file, these places are called 'sections'. The most
 * important types of sections are:
 * - text: for executable code;
 * - data: contains space for and values of statically allocated (global) variables, which are not
 *         initialized to zero;
 * - bss:  similar to data sections, but for variables that are initialized to zero.
 *
 * The main purpose of the bss section is to save space: there is no need to store all the zeros
 * in the ELF file -- they can be filled in at run-time, just before starting the application. When
 * running an application on Linux, the 'ELF loader' (part of the OS) will construct a memory image
 * image based on the code, data and metadata in its ELF file. It will zero-initialize the space
 * meant for variables from the bss section, and set up memory protection.
 *
 * The hypervisor runs bare-metal and the memory image is constructed upfront (.bin file produced
 * with objcopy). The remainder of the tasks of the loader must be performed by the hypervisor:
 * - initialize bss section(s) to zero (see startup.S);
 * - set up memory protection.
 *
 * The memory layout of the hypervisor is defined in the linker script. The linker uses this script
 * to arrange (and merge) the sections from all object files in the final ELF file.
 *
 * IMPORTANT: if a section is generated by the compiler that is not listed in the linker script,
 *            the linker will place it where it sees fit for that particular section!
 *
 * To protect hypervisor memory, these ELF sections must be mapped with attributes that reflect
 * their purpose:
 * - Text sections must be executable and must be read-only;
 * - Data sections must not be executable and must be either read-only or read/write.
 * - BSS sections must not be executable and must be read/write.
 *
 * The basic layout of the hypervisor in the ELF file consists of one text section (.text), a read-
 * only data section (.rodata), a read/write data section (.data) and a BSS section (.bss). Note
 * that .data and .bss have the same 'protection' attributes (not executable, read/write).
 *
 * The following 2 macros expose the starting and ending address of the hypervisor's first and last
 * ELF sections.
 */
#define HYPERVISOR_BEGIN_ADDRESS        ((u32int)&(__HYPERVISOR_BEGIN__))
#define HYPERVISOR_END_ADDRESS          ((u32int)&(__HYPERVISOR_END__))
/*
 * The following 2 macros expose the starting and ending address of all executable code. Because of
 * the way the linker script is designed, any extra sections inserted between .text and .rodata by
 * the linker will also be included in this range. GCC may generate a '.text.unlikely' section in
 * which cold code (e.g. startup code) is grouped to improve locality of the remainder of the code.
 */
#define HYPERVISOR_TEXT_BEGIN_ADDRESS   ((u32int)&(__TEXT_BEGIN__))
#define HYPERVISOR_TEXT_END_ADDRESS     ((u32int)&(__TEXT_END__))
/*
 * The following 4 macros expose the starting and ending address of statically allocated variables.
 * Part of these are read-only (.rodata), the rest is read/write (.data and .bss), but none of it
 * must be executable.
 */
#define HYPERVISOR_RO_XN_BEGIN_ADDRESS  ((u32int)&(__RODATA_BEGIN__))
#define HYPERVISOR_RO_XN_END_ADDRESS    ((u32int)&(__RODATA_END__))
#define HYPERVISOR_RW_XN_BEGIN_ADDRESS  ((u32int)&(__DATA_BEGIN__))
#define HYPERVISOR_RW_XN_END_ADDRESS    ((u32int)&(__BSS_END__))
/*
 * The remainder of the (hypervisor) memory is used for dynamic memory allocation. We make 2 pools:
 * - an executable pool for translated guest code (C$, only used with CONFIG_BLOCK_COPY);
 * - a non-executable pool for malloc(...).
 * The size and alignment of these pools is specified in the linker script. The next 4 macros
 * expose their starting and ending address.
 */
#define RAM_CODE_CACHE_POOL_BEGIN       ((u32int)&(__RAM_CODE_CACHE_POOL_BEGIN__))
#define RAM_CODE_CACHE_POOL_END         ((u32int)&(__RAM_CODE_CACHE_POOL_END__))
#define RAM_XN_POOL_BEGIN               ((u32int)&(__RAM_XN_POOL_BEGIN__))
#define RAM_XN_POOL_END                 ((u32int)&(__RAM_XN_POOL_END__))
/*
 * The symbols below are generated by the linker, used to determine the start and end of each ELF
 * section, and the calculated addresses for the pools for dynamic memory allocation.
 *
 * WARNING: do NOT use them except through the macros above!
 */
extern const u32int __HYPERVISOR_BEGIN__;
extern const u32int __TEXT_BEGIN__;
extern const u32int __TEXT_END__;
extern const u32int __RODATA_BEGIN__;
extern const u32int __RODATA_END__;
extern const u32int __DATA_BEGIN__;
extern const u32int __DATA_END__;
extern const u32int __BSS_BEGIN__;
extern const u32int __BSS_END__;
extern const u32int __HYPERVISOR_END__;
extern const u32int __SPILL_PAGE_BEGIN__;
extern const u32int __SPILL_PAGE_END__;
extern const u32int __RAM_CODE_CACHE_POOL_BEGIN__;
extern const u32int __RAM_CODE_CACHE_POOL_END__;
extern const u32int __RAM_XN_POOL_BEGIN__;
extern const u32int __RAM_XN_POOL_END__;

#endif /* __COMMON_LINKER_H__ */
