/* syscall.h -- Syscall wrapperes */

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

#ifndef __RENZAN_OSRT_SYSCALL_H__
#define __RENZAN_OSRT_SYSCALL_H__

#include <osrt/cdefs.h>

#define __OSRT_SYSCALL(name) __OSRT_PF_MNAME(SYSCALL_##name)

enum __OSRT_SYSCALLS {
        __OSRT_SYSCALL(RESV) = 0,
	__OSRT_SYSCALL(CAP_RETYPE),
	__OSRT_SYSCALL(CAP_MINT),
	__OSRT_SYSCALL(CAP_REVOKE_CHILDS),
	__OSRT_SYSCALL(CAP_DELETE),
        __OSRT_SYSCALL(AS_CREATE),
        __OSRT_SYSCALL(AS_CLONE),
        __OSRT_SYSCALL(AS_DESTROY),
        __OSRT_SYSCALL(MMAP),
        __OSRT_SYSCALL(MTRANSFER),
        __OSRT_SYSCALL(MUNMAP),
        __OSRT_SYSCALL(PORT_CREATE),
        __OSRT_SYSCALL(PORT_OPEN),
        __OSRT_SYSCALL(PORT_CLOSE),
        __OSRT_SYSCALL(PORT_REQUEST),
        __OSRT_SYSCALL(PORT_RECEIVE),
        __OSRT_SYSCALL(PORT_RESPONSE),
        __OSRT_SYSCALL(THREAD_SPAWN),
        __OSRT_SYSCALL(THREAD_EXIT),
        __OSRT_SYSCALL(THREAD_WAIT),
        __OSRT_SYSCALL(REINCARNATE),
        __OSRT_SYSCALL(FUTEX_WAIT),
        __OSRT_SYSCALL(FUTEX_WAKE),
        __OSRT_SYSCALL(COUNT)
};

#ifdef __RZ_OSRT

#include <stddef.h>
#include <stdint.h>

#include <osrt/futex.h>
#include <osrt/kobject.h>
#include <osrt/port.h>
#include <osrt/thread.h>

/* XXX to be removed */
#define AS_CURRENT 0

kobject_t syscall_as_create(void);
kobject_t syscall_as_clone(kobject_t address_space);
int64_t   syscall_as_destroy(kobject_t address_space);
intptr_t  syscall_mmap(
     kobject_t address_space, uintptr_t vaddr, size_t size, unsigned int flag);
kobject_t syscall_mtransfer(
    kobject_t dst, kobject_t src, uintptr_t vaddr_dst, uintptr_t vaddr_src,
    size_t size, unsigned int flag);
int64_t   syscall_munmap(kobject_t address_space, uintptr_t vaddr, size_t size);
kobject_t syscall_port_create(char *name, size_t namelen);
kobject_t syscall_port_open(char *name, size_t namelen);
int64_t   syscall_port_close(kobject_t port);
int64_t   syscall_port_request(kobject_t port, port_request_t *request);
int64_t   syscall_port_receive(
      kobject_t port, port_request_t *request_buffer, void *data_buffer,
      size_t buffer_size);
int64_t syscall_port_response(
    kobject_t request, uint64_t retval, void *ret_data, size_t ret_data_size);
__osrt_tid_t syscall_thread_spawn(kobject_t address_space, void *entry_point);
void         syscall_thread_exit(uint64_t retval) __attribute__((noreturn));
int64_t      syscall_thread_wait(tid_t tid, thread_state_t *state);
int64_t      syscall_reincarnate(kobject_t address_space, void *entry)
    __attribute__((noreturn));
void syscall_futex_wait(void *addr, __osrt_futex_val_t val);
void syscall_futex_wake(void *addr, size_t wake_count);

#endif /* __RZ_OSRT */

#endif /* __RENZAN_OSRT_SYSCALL_H__ */
