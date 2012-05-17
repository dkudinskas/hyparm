#ifndef __COMMON__COMPILER_H__
#define __COMMON__COMPILER_H__

/*
 * Header for overly compiler-specific things
 */

/*
 * __cold__
 * Hint the compiler that a specific function is unlikely to be executed. It will be put in a
 * separate section, .text.unlikely, so that hot code is put together (better cache behavior).
 */
#define __cold__      __attribute__((cold))

#define __constant__  __attribute__((const))
#define __pure__      __attribute__((pure))

/*
 * __macro__
 * Make a function definition behave like a macro. The definition is used only for inlining. In no
 * case is the function compiled as a standalone function, not even if you take its address
 * explicitly. Such an address becomes an external reference, as if you had only declared the
 * function, and had not defined it. The function definition should be placed in a header file.
 */
#define __macro__  extern inline __attribute__((always_inline,gnu_inline))

#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

#ifdef __GNUC__
#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
#define COMPILER_CAN_HIDE_WARNINGS
#define COMPILER_HAS_GCC_VECTOR_TYPES
#define COMPILER_HAS_STATIC_ASSERT
#endif
#endif

#endif /* __COMMON__COMPILER_H__ */
