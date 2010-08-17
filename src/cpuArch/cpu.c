/*
#include <common.h>
#include <command.h>
#include <asm/system.h>
#include <asm/cache.h>
#ifndef CONFIG_L2_OFF
#include <asm/arch/sys_proto.h>
#endif
*/
#include "cpu.h"

static inline unsigned int get_cr(void);
static inline void set_cr(unsigned int val);
static void cp_delay(void);
static void cache_enable(u32int cache_bit);
static void cache_disable(u32int cache_bit);
static void cache_flush(void);

extern void v7_flush_dcache_all(u32int dev);

/*
 * CPU specific code
 */

/***
 * we have no support for interrupts just yet
 ***/
void enable_interrupts()
{
  asm volatile("mrs %0, cpsr\n\t"
               "bic %0, %0, #0x80\n\t"
               "msr cpsr, %0 "
  ::"r"(0));
}
void disable_interrupts()
{
  asm volatile("mrs %0, cpsr\n\t"
               "orr %0, %0, #0x80\n\t"
               "msr cpsr, %0 "
  ::"r"(0));
}

/***
 * CACHE functions
 ***/
/* cache_bit must be either CR_I or CR_C */
static inline unsigned int get_cr(void)
{
  unsigned int val;
  asm("mrc p15, 0, %0, c1, c0, 0  @ get CR" : "=r" (val) : : "cc");
  return val;
}

static inline void set_cr(unsigned int val)
{
  asm volatile("mcr p15, 0, %0, c1, c0, 0  @ set CR"
    : : "r" (val) : "cc");
  isb();
}

static void cp_delay()
{
  volatile int i;

  /* copro seems to need some delay between reading and writing */
  for (i = 0; i < 100; i++)
    nop();
}

static void cache_enable(u32int cache_bit)
{
  u32int reg;

  reg = get_cr();  /* get control reg. */
  cp_delay();
  set_cr(reg | cache_bit);
}

/* cache_bit must be either CR_I or CR_C */
static void cache_disable(u32int cache_bit)
{
  u32int reg;

  reg = get_cr();
  cp_delay();
  set_cr(reg & ~cache_bit);
}

static void cache_flush()
{
  asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (0));
}

void icache_enable()
{
  cache_enable(CR_I);
}

void icache_disable()
{
  cache_disable(CR_I);
}

void dcache_enable()
{
  cache_enable(CR_C);
}

void dcache_disable()
{
  cache_disable(CR_C);
}

void l2_cache_enable()
{
  unsigned long i;
  __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
  __asm__ __volatile__("orr %0, %0, #0x2":"=r"(i));
  __asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));
}

void l2_cache_disable()
{
  unsigned long i;

  __asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
  __asm__ __volatile__("bic %0, %0, #0x2":"=r"(i));
  __asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));
}

int cleanup_before_linux()
{
  unsigned int i;

  /*
   * this function is called just before we call linux
   * it prepares the processor for linux
   *
   * we turn off caches etc ...
   */
  disable_interrupts();

  /* turn off I/D-cache */
  //icache_disable();
  //dcache_disable();

  /* invalidate I-cache */
  cache_flush();

  /* turn off L2 cache */
  l2_cache_disable();
  /* invalidate L2 cache also */
  v7_flush_dcache_all(BOARD_DEVICE_TYPE);

  i = 0;
  /* mem barrier to sync up things */
  asm("mcr p15, 0, %0, c7, c10, 4": :"r"(i));

  l2_cache_enable();
  enable_interrupts();
  return 0;
}
