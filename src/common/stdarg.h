#ifndef __COMMON__STDARG_H__
#define __COMMON__STDARG_H__ 1

typedef char *va_list;

#define ALIGNBND            (sizeof (signed int) - 1)
#define bnd(X, bnd)         (((sizeof (X)) + (bnd)) & (~(bnd)))

#define va_start(ap, last)  (void) ((ap) = (((char *) &(last)) + (bnd (last,ALIGNBND))))

#define va_arg(ap, type)    (*(type *)(((ap) += (bnd (type, ALIGNBND))) - (bnd (type,ALIGNBND))))

#define va_end(ap)          (void) 0

#endif
