#ifndef __CPU_ARCH__ARM_V7_H__
#define __CPU_ARCH__ARM_V7_H__

#include "common/types.h"

#define BOARD_DEVICE_TYPE        3


/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M  (1 << 0)  /* MMU enable        */
#define CR_A  (1 << 1)  /* Alignment abort enable    */
#define CR_C  (1 << 2)  /* Dcache enable      */
#define CR_W  (1 << 3)  /* Write buffer enable      */
#define CR_P  (1 << 4)  /* 32-bit exception handler    */
#define CR_D  (1 << 5)  /* 32-bit data address range    */
#define CR_L  (1 << 6)  /* Implementation defined    */
#define CR_B  (1 << 7)  /* Big endian        */
#define CR_S  (1 << 8)  /* System MMU protection    */
#define CR_R  (1 << 9)  /* ROM MMU protection      */
#define CR_F  (1 << 10)  /* Implementation defined    */
#define CR_Z  (1 << 11)  /* Implementation defined    */
#define CR_I  (1 << 12)  /* Icache enable      */
#define CR_V  (1 << 13)  /* Vectors relocated to 0xffff0000  */
#define CR_RR  (1 << 14)  /* Round Robin cache replacement  */
#define CR_L4  (1 << 15)  /* LDR pc can set T bit      */
#define CR_DT  (1 << 16)
#define CR_IT  (1 << 18)
#define CR_ST  (1 << 19)
#define CR_FI  (1 << 21)  /* Fast interrupt (lower latency mode)  */
#define CR_U  (1 << 22)  /* Unaligned access operation    */
#define CR_XP  (1 << 23)  /* Extended page tables      */
#define CR_VE  (1 << 24)  /* Vectored interrupts      */
#define CR_EE  (1 << 25)  /* Exception (Big) Endian    */
#define CR_TRE  (1 << 28)  /* TEX remap enable      */
#define CR_AFE  (1 << 29)  /* Access flag enable      */
#define CR_TE  (1 << 30)  /* Thumb exception enable    */




/*
 * This is used to ensure the compiler did actually allocate the register we
 * asked it for some inline assembly sequences.  Apparently we can't trust
 * the compiler from one version to another so a bit of paranoia won't hurt.
 * This string is meant to be concatenated with the inline asm string and
 * will cause compilation to stop on mismatch.
 * (for details, see gcc PR 15089)
 */
#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

#define isb() __asm__ __volatile__ ("" : : : "memory")

/*
 * NOP must be encoded as 'MOV r0,r0' in ARM code and 'MOV r8,r8' in Thumb code, see ARMv7-A/R ARM C.2
 */
#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");



int cleanupBeforeBoot(void);

void cache_flush(void);

void icache_enable(void);
void icache_disable(void);
void dcache_enable(void);
void dcache_disable(void);
void l2_cache_enable(void);
void l2_cache_disable(void);

#define iCacheFlushByMVA(vAddress) \
  { \
    __asm__ __volatile__ ("MCR p15, 0, %0, c7, c5, 1": :"r"(vAddress)); \
  }


#define enableInterrupts() \
  { \
    __asm__ __volatile__ ("CPSIE i"); \
  }

#define disableInterrupts() \
  { \
    __asm__ __volatile__ ("CPSID i"); \
  }

#define enableFastInterrupts() \
  { \
    __asm__ __volatile__ ("CPSIE f"); \
  }

#define disableFastInterrupts() \
  { \
    __asm__ __volatile__ ("CPSID f"); \
  }

/*
 * breakIfDebugging: insert software breakpoint when CONFIG_BKPT is set
 * infiniteIdleLoop: infinite loop waiting for interrupts (even if they are masked); used on crash
 */
#ifdef CONFIG_BKPT
#define breakIfDebugging() \
  { \
    __asm__ __volatile__ ("BKPT #0xBAD"); \
  }
#define infiniteIdleLoop() \
  { \
    while (TRUE) \
    { \
      __asm__ __volatile__ ("BKPT #0xBAD"); \
    } \
  }
#else
#define breakIfDebugging()  {}
#define infiniteIdleLoop() \
  { \
    while (TRUE) \
    { \
      __asm__ __volatile__ ("WFI"); \
    } \
  }
#endif /* CONFIG_BKPT */

#endif /* __CPU_ARCH__ARM_V7_H__ */
