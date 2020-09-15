/* memory.h -- memory management */
#ifndef ORIHIME_LIBOS_MEMORY_H_
#define ORIHIME_LIBOS_MEMORY_H_

#include <kern/memory.h>
#include <stddef.h>

#define PAGE_ATTR_WRITABLE   __MMAP_WRITABLE
#define PAGE_ATTR_EXECUTABLE __MMAP_EXECUTABLE

void *map_page(void *addr, size_t size, int flags);
void unmap_page(void *addr, size_t size);

#endif /* ORIHIME_LIBOS_MEMORY_H_ */
