#ifndef __COMMON__STDDEF_H__
#define __COMMON__STDDEF_H__

#define NULL                    ((void *)0)

#define offsetof(type, member)  __builtin_offsetof(type, member)

#endif
