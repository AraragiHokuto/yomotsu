#ifndef CALLS_H__
#define CALLS_H__

#include <kern/kobject.h>
#include <kern/port.h>
#include <stddef.h>
#include <stdint.h>

void print(const char *str);

int64_t as_create(void);
int64_t as_clone(kobject_handler_t as);
int64_t as_destroy(kobject_handler_t as);

int64_t mmap(kobject_handler_t as, uintptr_t addr, size_t size, uint64_t flag);
int64_t mshare(
    kobject_handler_t das, kobject_handler_t sas, uintptr_t vaddr_d,
    uintptr_t vaddr_s, size_t size, uint64_t flag);
int64_t munmap(kobject_handler_t as, uintptr_t addr, size_t size);

int64_t port_create(char *name, unsigned long namelen);
int64_t port_open(char *name, unsigned long namelen);
int64_t port_close(kobject_handler_t handler);
int64_t port_request(kobject_handler_t handler, port_request_t *req);
int64_t port_receive(
    kobject_handler_t handler, port_request_t *req, void *buf, size_t buflen);
int64_t port_response(
    kobject_handler_t handler, uint64_t retval_small, void *retval,
    size_t retval_size);
int64_t process_spawn(kobject_handler_t as, uint64_t entry);
int64_t process_wait(pid_t pid, process_state_t *state);
int64_t process_exit(uint64_t retval);

#endif /* CALLS_H__ */
