/* arch/amd64/hal_asm.h -- AMD64 assembly routines */

/*
 * Copyright 2022 Tenhouin Youkou <youkou@tenhou.in>.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __RENZAN_ARCH_AMD64_HAL_ASM_H__
#define __RENZAN_ARCH_AMD64_HAL_ASM_H__

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

#endif /* __RENZAN_ARCH_AMD64_HAL_ASM_H__ */
