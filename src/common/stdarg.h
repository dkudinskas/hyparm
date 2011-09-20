#ifndef __COMMON__STDARG_H__
#define __COMMON__STDARG_H__


#ifdef __GNUC__

/*
 * GCC's stdarg.h is the only valid and well defined one for GCC; anything else unless it uses the
 * builtins is broken.
 *
 * See GCC bug #48797 @ http://gcc.gnu.org/bugzilla/show_bug.cgi?id=48797
 */

typedef __builtin_va_list va_list;

#define va_start(v, l)  __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)

#define va_arg(v,l)     __builtin_va_arg(v,l)

#else

#error "Make sure you understand what you are doing and make a specific case for your compiler!"

typedef char *va_list;

#define ALIGNBND            (sizeof(signed int) - 1)
#define bnd(X, bnd)         (((sizeof(X)) + (bnd)) & (~(bnd)))

#define va_start(ap, last)  (void) ((ap) = (((char *) &(last)) + (bnd (last,ALIGNBND))))
#define va_end(ap)          (void) 0

#define va_arg(ap, type)                                                                           \
  (                                                                                                \
    *(type *)                                                                                      \
    (                                                                                              \
      (                                                                                            \
        (ap) += (bnd(type, ALIGNBND) + ((u32int)ap & (sizeof(type) - 1)))                          \
      )                                                                                            \
      - (bnd(type,ALIGNBND))                                                                       \
    )                                                                                              \
  )

#endif /* __GNUC__ */


#endif
