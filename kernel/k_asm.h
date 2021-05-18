#ifndef __RENZAN_K_ASM_H__
#define __RENZAN_K_ASM_H__

#include <osrt/types.h>

static inline u64
rdmsr(u64 msr)
{
        u32 l, h;
        asm volatile("rdmsr" : "=a"(l), "=d"(h) : "c"(msr));

        return (u64)h << 32 | l;
}

static inline void
wrmsr(u64 msr, u64 val)
{
        asm volatile("wrmsr" ::"a"(val & 0xffffffff), "d"(val >> 32), "c"(msr));
}

static inline void
outb(u16 port, u8 val)
{
        asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
}

static inline u8
inb(u16 port)
{
        u8 ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

static inline void
cpuid(u32 page, u32 *a, u32 *b, u32 *c, u32 *d)
{
        asm volatile("cpuid"
                     : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                     : "a"(page));
}

#endif /* __RENZAN_K_ASM_H__ */
