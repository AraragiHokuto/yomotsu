#include <kern/syscall.h>
#include <libos/syscall.h>
#include <stdint.h>

int64_t __do_syscall(
    uint64_t syscall_no, uint64_t arg1, uint64_t arg2, uint64_t arg3,
    uint64_t arg4, uint64_t arg5, uint64_t arg6);

kobject_t
syscall_as_create(void)
{
        return __do_syscall(SYSCALL_AS_CREATE, 0, 0, 0, 0, 0, 0);
}

kobject_t
syscall_as_clone(kobject_t address_space)
{
        return __do_syscall(SYSCALL_AS_CLONE, address_space, 0, 0, 0, 0, 0);
}

int64_t
syscall_as_destroy(kobject_t address_space)
{
        return __do_syscall(SYSCALL_AS_DESTROY, address_space, 0, 0, 0, 0, 0);
}

kobject_t
syscall_mmap(
    kobject_t address_space, uintptr_t vaddr, size_t size, unsigned int flag)
{
        return __do_syscall(
            SYSCALL_MMAP, address_space, vaddr, size, flag, 0, 0);
}

kobject_t
syscall_mtransfer(
    kobject_t dst, kobject_t src, uintptr_t vaddr_dst, uintptr_t vaddr_src,
    size_t size, unsigned int flag)
{
        return __do_syscall(
            SYSCALL_MTRANSFER, dst, src, vaddr_dst, vaddr_src, size, flag);
}

int64_t
syscall_munmap(kobject_t address_space, uintptr_t vaddr, size_t size)
{
        return __do_syscall(
            SYSCALL_MUNMAP, address_space, vaddr, size, 0, 0, 0);
}

kobject_t
syscall_port_create(char *name, size_t namelen)
{
        return __do_syscall(
            SYSCALL_PORT_CREATE, (uintptr_t)name, namelen, 0, 0, 0, 0);
}

kobject_t
syscall_port_open(char *name, size_t namelen)
{
        return __do_syscall(
            SYSCALL_PORT_OPEN, (uintptr_t)name, namelen, 0, 0, 0, 0);
}

int64_t
syscall_port_close(kobject_t port)
{
        return __do_syscall(SYSCALL_PORT_CLOSE, port, 0, 0, 0, 0, 0);
}

int64_t
syscall_port_request(kobject_t port, port_request_t *request)
{
        return __do_syscall(
            SYSCALL_PORT_REQUEST, port, (uintptr_t)request, 0, 0, 0, 0);
}

int64_t
syscall_port_receive(
    kobject_t port, port_request_t *buffer, void *buf, size_t buflen)
{
        return __do_syscall(
            SYSCALL_PORT_RECEIVE, port, (uintptr_t)buffer, (uintptr_t)buf,
            buflen, 0, 0);
}

int64_t
syscall_port_response(
    kobject_t request, int64_t retval, void *ret_data, size_t ret_data_size)
{
        return __do_syscall(
            SYSCALL_PORT_RESPONSE, request, retval, (uintptr_t)ret_data,
            ret_data_size, 0, 0);
}

pid_t
syscall_process_spawn(kobject_t address_space, void *entry_point)
{
        return __do_syscall(
            SYSCALL_PROCESS_SPAWN, address_space, (uintptr_t)entry_point, 0, 0,
            0, 0);
}

void __attribute__((noreturn)) syscall_process_exit(uint64_t retval)
{
        __do_syscall(SYSCALL_PROCESS_EXIT, retval, 0, 0, 0, 0, 0);
        /* dear compiler: we really don't return */
        while (1)
                ;
}

int64_t
syscall_process_wait(pid_t pid, process_state_t *state)
{
        return __do_syscall(
            SYSCALL_PROCESS_WAIT, pid, (uintptr_t)state, 0, 0, 0, 0);
}

int64_t __attribute__((noreturn))
syscall_reincarnate(kobject_t address_space, void *entry)
{
        __do_syscall(
            SYSCALL_REINCARNATE, address_space, (uintptr_t)entry, 0, 0, 0, 0);
        /* dear compiler: we really don't return */
        while (1)
                ;
}

void
syscall_futex_wait(void *addr, futex_val_t val)
{
        __do_syscall(SYSCALL_FUTEX_WAIT, (uintptr_t)addr, val, 0, 0, 0, 0);
}

void
syscall_futex_wake(void *addr, size_t count)
{
        __do_syscall(SYSCALL_FUTEX_WAIT, (uintptr_t)addr, count, 0, 0, 0, 0);
}
