/* _crt.c -- CRI ABI */

#include <libos/process.h>
#include <libos/syscall.h>
#include <stdint.h>

void
__libc_init(void)
{
}

__attribute__((noreturn)) void
__exit(int64_t retval)
{
        syscall_process_exit(retval);
}

int main(int argc, char **argv);

void
__main_wrapper(void)
{
        int ret = main(__INIT_DATA->argc, __INIT_DATA->argv);
        __exit(ret);
}
