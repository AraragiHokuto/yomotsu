#ifndef CALLS_H__
#define CALLS_H__

#include <kern/kobject.h>
#include <kern/port.h>

void	print(const char* str);

s64	as_create(void);
s64	as_clone(kobject_handler_t as);
s64	as_destroy(kobject_handler_t as);

s64	mmap(kobject_handler_t as, uintptr addr, size_t size, uint flag);
s64	mshare(kobject_handler_t das, kobject_handler_t sas,
	       uintptr vaddr_d, uintptr vaddr_s, size_t size, uint flag);
s64	munmap(kobject_handler_t as, uintptr addr, size_t size);

s64	port_create(char *name, unsigned long namelen);
s64	port_open(char *name, unsigned long namelen);
s64	port_close(kobject_handler_t handler);
s64	port_request(kobject_handler_t handler, port_request_t *req);
s64	port_receive(kobject_handler_t handler, port_request_t *req,
			     void *buf, size_t buflen);
s64	port_response(kobject_handler_t handler,
		      u64 retval_small,	void *retval, size_t retval_size);
s64	process_spawn(kobject_handler_t as, u64 entry);
s64	process_wait(pid_t pid, process_state_t *state);
s64	process_exit(u64 retval);

#endif	/* CALLS_H__ */
