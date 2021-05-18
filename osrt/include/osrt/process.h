/* osrt/process.h -- OSRT Process definitions */

/*
 * Copyright 2021 Mosakuji Hokuto <shikieiki@yamaxanadu.org>.
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

#ifndef __RENZAN_OSRT_PROCESS_H__
#define __RENZAN_OSRT_PROCESS_H__

#include <osrt/types.h>

typedef __osrt_u64 __osrt_pid_t;

struct process_state_s {
        __osrt_pid_t process_id;
        __osrt_u64 retval;
};

/* XXX to be removed */
typedef struct process_state_s process_state_t;

/* XXX to be removed */
typedef __osrt_pid_t pid_t;

#include <stdbool.h>
#include <stdint.h>

#include <osrt/memory.h>

__osrt_pid_t process_spawn_from_memory(void *elf, int argc, char **argv);
bool         process_wait(__osrt_pid_t pid, __osrt_s64 *ret);

// pid_t process_spawn_from_memory(void *executable)
_Noreturn void process_exit(__osrt_s64 retval);

#ifdef __RZ_OSRT

/* init data -- always resides at the end of userspace */
struct __libos_process_init_data_s {
        int    argc;
        char **argv;

        /* rsp always at 0x7ffffffffffffff8 */
        uintptr_t rsp;
};

typedef struct __libos_process_init_data_s __libos_process_init_data_t;

#define __USERLAND_END_ADDR 0x800000000000
#define __LASTPAGE_ADDR     (__USERLAND_END_ADDR - __OSRT_PAGE_SIZE)

#define __INIT_DATA                   \
        ((__libos_process_init_data_t \
              *)((uint8_t *)__USERLAND_END_ADDR - sizeof(__libos_process_init_data_t)))

#define __INIT_DATA_BY_PAGE(page)     \
        ((__libos_process_init_data_t \
              *)((uint8_t *)page + __PAGE_SIZE - sizeof(__libos_process_init_data_t)))

#endif /* __RZ_OSRT */


#ifdef __RZ_KERNEL

typedef __osrt_pid_t pid_t;

#endif /* __RZ_KERNEL */

#endif /* __RENZAN_OSRT_PROCESS_H__ */
