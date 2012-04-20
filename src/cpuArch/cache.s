/*
 *  v7_flush_dcache_all()
 *
 *  Flush the whole D-cache.
 *
 */
  .align 5
.global v7_flush_dcache_all
v7_flush_dcache_all:
  push    {r4, r5, r7, r9, sl, fp, lr}
  mrc     p15, 1, r0, cr0, cr0, 1
  ands    r3, r0, #0x7000000
  lsr     r3, r3, #23
  beq     finished
  mov     sl, #0

loopCache1:
  add     r2, sl, sl, lsr #1
  lsr     r1, r0, r2
  and     r1, r1, #7
  cmp     r1, #2
  blt     skip
  mcr     p15, 2, sl, cr0, cr0, 0
  isb     sy
  mrc     p15, 1, r1, cr0, cr0, 0
  and     r2, r1, #7
  add     r2, r2, #4
  ldr     r4, [pc, #92]
  ands    r4, r4, r1, lsr #3
  clz     r5, r4
  ldr     r7, [pc, #84] 
  ands    r7, r7, r1, lsr #13

loopCache2:
  mov     r9, r4

loopCache3:
  orr     fp, sl, r9, lsl r5
  orr     fp, fp, r7, lsl r2
  mcr     p15, 0, fp, cr7, cr14, 2
  subs    r9, r9, #1
  bge     loopCache3
  subs    r7, r7, #1
  bge     loopCache2

skip:
  add     sl, sl, #2
  cmp     r3, sl
  bgt     loopCache1

finished:
  mov     sl, #0
  mcr     p15, 2, sl, cr0, cr0, 0
  isb     sy

  mov     r0, #0
  mcr     p15, 0, r0, cr7, cr5, 0
  pop     {r4, r5, r7, r9, sl, fp, lr}
  mov     pc, lr
  nop
  nop
  
var1:
  .word 0x000003ff
var2:
  .word 0x00007fff
