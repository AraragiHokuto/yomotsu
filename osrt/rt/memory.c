/* XXX to be reimplemented */

#include <errno.h>
#include <stdint.h>

#include <osrt/syscall.h>

void *
map_page(void *addr, size_t size, int flags)
{
        int64_t ret = syscall_mmap(0, (uintptr_t)addr, size, flags);
        if (ret < 0) {
                errno = -ret;
                return NULL;
        }
        return addr;
}

void
unmap_page(void *addr, size_t size)
{
        int64_t ret = syscall_munmap(0, (uintptr_t)addr, size);
        if (ret < 0) { errno = -ret; }
}
