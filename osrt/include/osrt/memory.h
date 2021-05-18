/* memory.h -- memory management */
#ifndef RENZAN_LIBOS_MEMORY_H_
#define RENZAN_LIBOS_MEMORY_H_

#include <stddef.h>

/* XXX: should move to platform headers */
#define __OSRT_PAGE_SIZE (2048 * 1024) /* 2MB */

enum __OSRT_PAGE_ATTR {
	__OSRT_PAGE_ATTR_WRITABLE = 2,
	__OSRT_PAGE_ATTR_EXECUTABLE = 8,
};

void *map_page(void *addr, size_t size, int flags);
void  unmap_page(void *addr, size_t size);

#endif /* RENZAN_LIBOS_MEMORY_H_ */
