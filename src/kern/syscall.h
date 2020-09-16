#ifndef ORIHIME_SYSCALL_H__
#define ORIHIME_SYSCALL_H__

enum SYSCALLS {
        SYSCALL_AS_CREATE = 0,
        SYSCALL_AS_CLONE,
        SYSCALL_AS_DESTROY,
        SYSCALL_MMAP,
        SYSCALL_MTRANSFER,
        SYSCALL_MUNMAP,
        SYSCALL_PORT_CREATE,
        SYSCALL_PORT_OPEN,
        SYSCALL_PORT_CLOSE,
        SYSCALL_PORT_REQUEST,
        SYSCALL_PORT_RECEIVE,
        SYSCALL_PORT_RESPONSE,
        SYSCALL_PROCESS_SPAWN,
        SYSCALL_PROCESS_EXIT,
        SYSCALL_PROCESS_WAIT,
        SYSCALL_REINCARNATE,
        SYSCALL_FUTEX_WAIT,
        SYSCALL_FUTEX_WAKE,
        __SYSCALL_COUNT
};

#define AS_CURRENT 0

#ifdef _KERNEL

#include <kern/types.h>

void syscall_def(void);
void syscall_init(void);

#endif /* _KERNEL */

#endif /* ORIHIME_SYSCALL_H__ */
