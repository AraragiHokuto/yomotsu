#ifndef KAGUYA_TYPES_H_
#define KAGUYA_TYPES_H_

#ifdef _KERNEL

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef u64 uint;

typedef signed char  s8;
typedef signed short s16;
typedef signed int   s32;
typedef signed long  s64;

typedef s64 sint;

typedef uint uintptr;
typedef sint sintptr;

typedef sint offset_t;
typedef uint size_t;

typedef u8 byte;

/* use reserved identifiers to avoid userland namespace pollution */

typedef u8  __u8;
typedef u16 __u16;
typedef u32 __u32;
typedef u64 __u64;

typedef uint __uint;

typedef s8  __s8;
typedef s16 __s16;
typedef s32 __s32;
typedef s64 __s64;

typedef sint __sint;

typedef uintptr __uintptr;
typedef sintptr __sintptr;

typedef byte __byte;

#else /* _KERNEL */

#include <stdint.h>

typedef uint8_t  __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;

typedef __u64 __uint;

typedef int8_t  __s8;
typedef int16_t __s16;
typedef int32_t __s32;
typedef int64_t __s64;

typedef __s64 __sint;

typedef uintptr_t __uintptr;
typedef intptr_t  __sintptr;

typedef __u8 __byte;

#endif /* _KERNEL */

#endif /* KAGUYA_TYPES_H_ */
