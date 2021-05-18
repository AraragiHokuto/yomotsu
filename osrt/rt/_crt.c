/* _crt.c -- CRI ABI */

#include <osrt/process.h>
#include <osrt/syscall.h>

#include <stdint.h>

typedef void (*__crt_func_ptr_t)(void);

extern __crt_func_ptr_t __preinit_array_start[];
extern __crt_func_ptr_t __preinit_array_end[];
extern __crt_func_ptr_t __init_array_start[];
extern __crt_func_ptr_t __init_array_end[];
extern __crt_func_ptr_t __fini_array_start[];
extern __crt_func_ptr_t __fini_array_end[];

void
__crt_preinit(void)
{
	for (__crt_func_ptr_t *f = __preinit_array_start;
	     f != __preinit_array_end; ++f) {
		(*f)();
	}
}

void
__crt_init(void)
{
	for (__crt_func_ptr_t* f = __init_array_start;
	     f != __init_array_end; ++f) {
		(*f)();
	}
}

void
__crt_fini(void)
{
	for (__crt_func_ptr_t *f = __fini_array_start;
	     f != __fini_array_end; ++f) {
		(*f)();
	}
}

__attribute__((noreturn)) void
__exit(int64_t retval)
{
	__crt_fini();
        syscall_process_exit(retval);
}

int main(int argc, char **argv);

void
__main_wrapper(void)
{
        int ret = main(__INIT_DATA->argc, __INIT_DATA->argv);
        __exit(ret);
}
