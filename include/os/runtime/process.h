/* process.h -- Process control */
#ifndef __ORIHIME_LIBOS_PROCESS_H__
#define __ORIHIME_LIBOS_PROCESS_H__

#include <os/kernel/process.h>
#include <stdint.h>
#include <stdbool.h>

pid_t process_spawn_from_memory(void *elf, int argc, char **argv);
bool  process_wait(pid_t pid, int64_t *ret);

// pid_t process_spawn_from_memory(void *executable)
_Noreturn void process_exit(int64_t retval);

#ifdef __ORIHIME_OSRT

#include <os/kernel/memory.h>

/* init data -- always resides at the end of userspace */
struct __libos_process_init_data_s {
        int    argc;
        char **argv;

        /* rsp always at 0x7ffffffffffffff8 */
        uintptr_t rsp;
};

typedef struct __libos_process_init_data_s __libos_process_init_data_t;

#define __USERLAND_END_ADDR  0x800000000000
#define __LASTPAGE_ADDR      (__USERLAND_END_ADDR - __PAGE_SIZE)

#define __INIT_DATA				\
        ((__libos_process_init_data_t *)(	\
		(uint8_t *)__USERLAND_END_ADDR	\
		- sizeof(__libos_process_init_data_t)))

#define __INIT_DATA_BY_PAGE(page)					\
        ((__libos_process_init_data_t *)(				\
		(uint8_t *)page + __PAGE_SIZE				\
		- sizeof(__libos_process_init_data_t)))

#endif /* __ORIHIME_OSRT */
#endif /* __ORIHIME_LIBOS_PROCESS_H__ */
