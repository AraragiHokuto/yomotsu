/* osrt/memory.h -- Memory management */
/* XXX to be reimplemented */

#ifndef __RENZAN_OSRT_MEMORY_H__
#define __RENZAN_OSRT_MEMORY_H__

#include <stddef.h>

/* XXX: should move to platform headers */
#define __OSRT_PAGE_SIZE (2048 * 1024) /* 2MB */

enum __OSRT_PAGE_ATTR {
        __OSRT_PAGE_ATTR_WRITABLE   = 2,
        __OSRT_PAGE_ATTR_EXECUTABLE = 8,
};

void *map_page(void *addr, size_t size, int flags);
void  unmap_page(void *addr, size_t size);

#endif /* __RENZAN_OSRT_MEMORY_H__ */
