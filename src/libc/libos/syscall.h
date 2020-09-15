/* syscall.h -- thin wrapper around kernel syscalls */
#ifndef ORIHIME_LIBOS_SYSCALL_H_
#define ORIHIME_LIBOS_SYSCALL_H_

#ifdef __ORIHIME_LIBOS_INTERNAL

#include <kern/futex.h>
#include <kern/memory.h>
#include <kern/port.h>
#include <kern/process.h>
#include <kern/syscall.h>
#include <kern/kobject.h>
#include <stddef.h>
#include <stdint.h>

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
kobject_t syscall_port_receive(
    kobject_t port, port_request_t *request_buffer, void *data_buffer,
    size_t buffer_size);
int64_t syscall_port_response(
    kobject_t request, int64_t retval, void *ret_data, size_t ret_data_size);
tid_t   syscall_thread_spawn(void *entry_point);
pid_t   syscall_process_spawn(kobject_t address_space, void *entry_point);
void    syscall_thread_exit(int64_t retval) __attribute__((noreturn));
void    syscall_process_exit(int64_t retval) __attribute__((noreturn));
int64_t syscall_thread_wait(tid_t tid, thread_state_t *state);
int64_t syscall_process_wait(pid_t pid, process_state_t *state);
int64_t syscall_reincarnate(kobject_t address_space, void *entry)
    __attribute__((noreturn));
void syscall_futex_wait(void *addr, futex_val_t val);
void syscall_futex_wake(void *addr, size_t wake_count);

#endif /* __ORIHIME_LIBOS_INTERNAL */

#endif /* ORIHIME_LIBOS_SYSCALL_H_ */
