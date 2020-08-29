#include "calls.h"

#include <kern/kobject.h>
#include <kern/syscall.h>
#include <stdint.h>

extern long __do_syscall(
    uint64_t no, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4,
    uint64_t arg5, uint64_t arg6);

int64_t
as_create(void)
{
        return __do_syscall(SYSCALL_AS_CREATE, 0, 0, 0, 0, 0, 0);
}

int64_t
as_clone(kobject_handler_t as)
{
        return __do_syscall(SYSCALL_AS_CLONE, as, 0, 0, 0, 0, 0);
}

int64_t
as_destroy(kobject_handler_t as)
{
        return __do_syscall(SYSCALL_AS_DESTROY, as, 0, 0, 0, 0, 0);
}

int64_t
mmap(kobject_handler_t as, uint64_t addr, uint64_t size, uint64_t flag)
{
        return __do_syscall(SYSCALL_MMAP, as, addr, size, flag, 0, 0);
}

int64_t
mtransfer(
    kobject_handler_t das, kobject_handler_t sas, uintptr_t vaddr_d,
    uintptr_t vaddr_s, size_t size, uint64_t flag)
{
        return __do_syscall(
            SYSCALL_MTRANSFER, das, sas, vaddr_d, vaddr_s, size, flag);
}

long
munmap(kobject_handler_t as, uintptr_t addr, size_t size)
{
        return __do_syscall(SYSCALL_MUNMAP, as, addr, size, 0, 0, 0);
}

long
port_create(char *name, unsigned long namelen)
{
        return __do_syscall(
            SYSCALL_PORT_CREATE, (uint64_t)name, namelen, 0, 0, 0, 0);
}

long
port_open(char *name, unsigned long namelen)
{
        return __do_syscall(
            SYSCALL_PORT_OPEN, (uint64_t)name, namelen, 0, 0, 0, 0);
}

long
port_close(kobject_handler_t handler)
{
        return __do_syscall(SYSCALL_PORT_CLOSE, handler, 0, 0, 0, 0, 0);
}

long
port_request(kobject_handler_t handler, port_request_t *req)
{
        return __do_syscall(
            SYSCALL_PORT_REQUEST, handler, (uint64_t)req, 0, 0, 0, 0);
}

long
port_receive(
    kobject_handler_t handler, port_request_t *req, void *buf, size_t buflen)
{
        return __do_syscall(
            SYSCALL_PORT_RECEIVE, handler, (uint64_t)req, (uint64_t)buf, buflen,
            0, 0);
}

long
port_response(
    kobject_handler_t handler, uint64_t retval_small, void *retval,
    size_t retval_size)
{
        return __do_syscall(
            SYSCALL_PORT_RESPONSE, handler, retval_small, (uint64_t)retval,
            retval_size, 0, 0);
}

long
process_spawn(kobject_handler_t as, uint64_t entry)
{
        return __do_syscall(SYSCALL_PROCESS_SPAWN, as, entry, 0, 0, 0, 0);
}

int64_t
process_wait(pid_t pid, process_state_t *state)
{
        return __do_syscall(
            SYSCALL_PROCESS_WAIT, pid, (uint64_t)state, 0, 0, 0, 0);
}

int64_t
process_exit(uint64_t retval)
{
        return __do_syscall(SYSCALL_PROCESS_EXIT, retval, 0, 0, 0, 0, 0);
}

void
print(const char *str)
{
        uint64_t len;
        for (len = 0; str[len]; ++len)
                ;

        port_request_t req;
        req.type        = PORT_REQ_TYPE_CUSTOM_START + 1; /* stub */
        req.data_addr   = (void *)str;
        req.data_size   = len;
        req.retval_addr = (void *)0;
        req.retval_size = 0;

        kobject_handler_t debug_port = port_open("kern.debug_print", 16);
        port_request(debug_port, &req);
        port_close(debug_port);
}
