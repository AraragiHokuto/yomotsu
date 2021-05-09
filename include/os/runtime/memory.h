/* memory.h -- memory management */
#ifndef RENZAN_LIBOS_MEMORY_H_
#define RENZAN_LIBOS_MEMORY_H_

#include <os/kernel/memory.h>

#include <stddef.h>

#define PAGE_ATTR_WRITABLE   __MMAP_WRITABLE
#define PAGE_ATTR_EXECUTABLE __MMAP_EXECUTABLE

void *map_page(void *addr, size_t size, int flags);
void unmap_page(void *addr, size_t size);

#endif /* RENZAN_LIBOS_MEMORY_H_ */
