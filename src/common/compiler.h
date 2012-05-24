#ifndef __COMMON__COMPILER_H__
#define __COMMON__COMPILER_H__

/*
 * Header for overly compiler-specific things (GCC-specific, that is).
 *
 *
 * __cold__ (function attribute)
 * Hint the compiler that a specific function is unlikely to be executed. It will be put in a
 * separate ELF text section, '.text.unlikely', so that hot code is put together (better cache
 * behavior). For more information on ELF sections, see linker.h.
 */
#define __cold__      __attribute__((cold))
/*
 * __constant__ (function attribute)
 * Hint the compiler that a function does not examine any values except its arguments, and has no
 * effects except the return value. The following rules are important when using this attribute:
 * - It does not make sense for a __constant__ function to return void.
 * - Functions marked __constant__ must not call non-__constant__ functions.
 * - For functions with pointer arguments, __constant__ implies that only the pointer values are
 *   used, and not the referenced memory.
 * This hint allows the compiler to perform better (more agressive) optimizations when calling
 * functions across object file boundaries.
 */
#define __constant__  __attribute__((const))
/*
 * __pure__ (function attribute)
 * Hint the compiler that a function has no effect other than the return value and its return value
 * only depends on parameters and/or global variables. Keep these rules in mind:
 * - No calls to non-pure and non-constant functions;
 * - There must be no possibility for infinite loops in the function body (this implies not a
 *   single call to ASSERT or DIE_NOW);
 * - No dependency on volatile memory or other system resource (this means pure functions may not
 *   call getActiveGuestContext).
 * This means that the function is SAFE TO CALL FEWER TIMES THAN THE PROGRAM SAYS.
 */
#define __pure__      __attribute__((pure))
/*
 * __macro__ (function attribute)
 * Make a function definition behave like a macro. The definition is used only for inlining. In no
 * case is the function compiled as a standalone function, not even if you take its address
 * explicitly. Such an address becomes an external reference, as if you had only declared the
 * function, and had not defined it. BOTH function declaration AND definition should be placed in a
 * header file.
 */
#define __macro__     extern inline __attribute__((always_inline,gnu_inline))
/*
 * __malloc__ (function attribute)
 * Hint the compiler that any non-NULL pointer returned by a function cannot alias any other
 * pointer valid when the function returns and that the memory has undefined content. This will
 * often improve optimization (generally applies to malloc, memalign and calloc, but not realloc).
 */
#define __malloc__    __attribute__((malloc))

#define likely(x)     __builtin_expect(!!(x), 1)
#define unlikely(x)   __builtin_expect(!!(x), 0)

#ifdef __GNUC__
#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
#define COMPILER_CAN_HIDE_WARNINGS
#define COMPILER_HAS_GCC_VECTOR_TYPES
#define COMPILER_HAS_STATIC_ASSERT
#endif
#endif

#endif /* __COMMON__COMPILER_H__ */
