/* syscall.h -- thin wrapper around kernel syscalls */
#ifndef __RENZAN_OSRT_SYSCALL_H__
#define __RENZAN_OSRT_SYSCALL_H__

enum __OSRT_SYSCALLS {
        __OSRT_SYSCALL_AS_CREATE = 0,
        __OSRT_SYSCALL_AS_CLONE,
        __OSRT_SYSCALL_AS_DESTROY,
        __OSRT_SYSCALL_MMAP,
        __OSRT_SYSCALL_MTRANSFER,
        __OSRT_SYSCALL_MUNMAP,
        __OSRT_SYSCALL_PORT_CREATE,
        __OSRT_SYSCALL_PORT_OPEN,
        __OSRT_SYSCALL_PORT_CLOSE,
        __OSRT_SYSCALL_PORT_REQUEST,
        __OSRT_SYSCALL_PORT_RECEIVE,
        __OSRT_SYSCALL_PORT_RESPONSE,
        __OSRT_SYSCALL_PROCESS_SPAWN,
        __OSRT_SYSCALL_PROCESS_EXIT,
        __OSRT_SYSCALL_PROCESS_WAIT,
        __OSRT_SYSCALL_REINCARNATE,
        __OSRT_SYSCALL_FUTEX_WAIT,
        __OSRT_SYSCALL_FUTEX_WAKE,
        __OSRT_SYSCALL_COUNT
};

#ifdef __RZ_KERNEL

/* TODO */

#endif /* __RZ_KERNEL */

#ifdef __RENZAN_OSRT

#include <stddef.h>
#include <stdint.h>

#include <osrt/futex.h>
#include <osrt/kobject.h>
#include <osrt/port.h>
#include <osrt/process.h>

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
__osrt_pid_t syscall_process_spawn(kobject_t address_space, void *entry_point);
void         syscall_process_exit(uint64_t retval) __attribute__((noreturn));
int64_t      syscall_process_wait(pid_t pid, process_state_t *state);
int64_t      syscall_reincarnate(kobject_t address_space, void *entry)
    __attribute__((noreturn));
void syscall_futex_wait(void *addr, __osrt_futex_val_t val);
void syscall_futex_wake(void *addr, size_t wake_count);

#endif /* __RENZAN_OSRT */

#endif /* __RENZAN_OSRT_SYSCALL_H_ */
