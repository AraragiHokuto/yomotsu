#include "calls.h"
#include <kern/kobject.h>
#include <kern/syscall.h>

extern long	__do_syscall(
	u64 no, u64 arg1, u64 arg2, u64 arg3,
	u64 arg4, u64 arg5, u64 arg6);

s64
as_create(void)
{
	return __do_syscall(SYSCALL_AS_CREATE, 0, 0, 0, 0, 0, 0);
}

s64
as_clone(kobject_handler_t as)
{
	return __do_syscall(SYSCALL_AS_CLONE, as, 0, 0, 0, 0, 0);
}

s64
as_destroy(kobject_handler_t as)
{
	return __do_syscall(SYSCALL_AS_DESTROY, as, 0, 0, 0, 0, 0);
}

long
mmap(kobject_handler_t as, u64 addr, u64 size, uint flag)
{
        return __do_syscall(SYSCALL_MMAP, as, addr, size, flag, 0, 0);
}

long
mtransfer(kobject_handler_t das, kobject_handler_t sas,
	  uintptr vaddr_d, uintptr vaddr_s, size_t size, uint flag)
{
	return __do_syscall(SYSCALL_MTRANSFER, das, sas,
			    vaddr_d, vaddr_s, size, flag);
}

long
munmap(kobject_handler_t as, uintptr addr, size_t size)
{
        return __do_syscall(SYSCALL_MUNMAP, as, addr, size, 0, 0, 0);
}

long
port_create(char *name, unsigned long namelen)
{
	return __do_syscall(SYSCALL_PORT_CREATE, (u64)name, namelen,
			    0, 0, 0, 0);
}

long
port_open(char *name, unsigned long namelen)
{
	return __do_syscall(SYSCALL_PORT_OPEN, (u64)name, namelen,
			    0, 0, 0, 0);
}

long
port_close(kobject_handler_t handler)
{
	return __do_syscall(SYSCALL_PORT_CLOSE, handler, 0, 0, 0, 0, 0);
}

long
port_request(kobject_handler_t handler, port_request_t *req)
{
	return __do_syscall(SYSCALL_PORT_REQUEST, handler, (u64)req,
			    0, 0, 0, 0);
}

long
port_receive(kobject_handler_t handler,
	     port_request_t *req,
	     void *buf, size_t buflen)
{
	return __do_syscall(SYSCALL_PORT_RECEIVE, handler,
			    (u64)req, (u64)buf, buflen, 0, 0);
}

long
port_response(kobject_handler_t handler,
	      u64 retval_small, void *retval, size_t retval_size)
{
	return __do_syscall(SYSCALL_PORT_RESPONSE, handler, retval_small,
			    (u64)retval, retval_size, 0, 0);
}

long
process_spawn(kobject_handler_t as, u64 entry)
{
	return __do_syscall(SYSCALL_PROCESS_SPAWN, as, entry, 0, 0, 0, 0);
}

s64
process_wait(pid_t pid, process_state_t *state)
{
	return __do_syscall(SYSCALL_PROCESS_WAIT, pid, (u64)state, 0, 0, 0, 0);
}

s64
process_exit(u64 retval)
{
	return __do_syscall(SYSCALL_PROCESS_EXIT, retval, 0, 0, 0, 0, 0);
}

void
print(const char *str)
{
	u64 len;
	for (len = 0; str[len]; ++len);

	port_request_t req;
	req.type	= PORT_REQ_TYPE_CUSTOM_START + 1; /* stub */
	req.data_addr	= (void*)str;
	req.data_size	= len;
	req.retval_addr	= (void*)0;
	req.retval_size	= 0;

	kobject_handler_t debug_port	= port_open("kern.debug_print", 16);
	port_request(debug_port, &req);
	port_close(debug_port);
}
