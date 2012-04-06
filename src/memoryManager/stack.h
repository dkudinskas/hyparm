#ifndef __MEMORY_MANAGER__STACK_H__
#define __MEMORY_MANAGER__STACK_H__

#include "common/compiler.h"
#include "common/types.h"


/*
 * Macros for stack gaps.
 */
#define ABT_STACK_GAP   ((u32int)&(abtStackGap))
#define FIQ_STACK_GAP   ((u32int)&(fiqStackGap))
#define IRQ_STACK_GAP   ((u32int)&(irqStackGap))
#define SVC_STACK_GAP   ((u32int)&(svcStackGap))
#define UND_STACK_GAP   ((u32int)&(undStackGap))
#define TOP_STACK_GAP   ((u32int)&(topStackGap))


/*
 * Macros for lower bound and upper bound for each stack.
 *
 * IMPORTANT: stacks are descending!
 */
#define ABT_STACK_LB    ((u32int)&(abtStackEnd))
#define ABT_STACK_UB    ((u32int)&(abtStack))
#define FIQ_STACK_LB    ((u32int)&(fiqStackEnd))
#define FIQ_STACK_UB    ((u32int)&(fiqStack))
#define IRQ_STACK_LB    ((u32int)&(irqStackEnd))
#define IRQ_STACK_UB    ((u32int)&(irqStack))
#define SVC_STACK_LB    ((u32int)&(svcStackEnd))
#define SVC_STACK_UB    ((u32int)&(svcStack))
#define UND_STACK_LB    ((u32int)&(undStackEnd))
#define UND_STACK_UB    ((u32int)&(undStack))


/*
 * Do NOT use the following symbols directly; use the macros above!
 */
extern const u32int abtStack;
extern const u32int abtStackEnd;
extern const u32int abtStackGap;

extern const u32int fiqStack;
extern const u32int fiqStackEnd;
extern const u32int fiqStackGap;

extern const u32int irqStack;
extern const u32int irqStackEnd;
extern const u32int irqStackGap;

extern const u32int svcStack;
extern const u32int svcStackEnd;
extern const u32int svcStackGap;

extern const u32int undStack;
extern const u32int undStackEnd;
extern const u32int undStackGap;

extern const u32int topStackGap;

#endif /* __MEMORY_MANAGER__STACK_H__ */
