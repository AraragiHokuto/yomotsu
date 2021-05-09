/* dlmalloc_renzan.inc.h -- Renzan dlmalloc configuration */

#define MSPACES       1
#define HAVE_MORECORE 0

#define LACKS_UNISTD_H
#define LACKS_FCNTL_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_SCHED_H
#define LACKS_TIME_H

#define USE_LOCKS 0

#include <os/runtime/memory.h>
#include <stddef.h> /* size_t */

#define malloc_getpagesize __PAGE_SIZE

/* mmap wrappers */
#define MMAP(s)        map_page(NULL, (s), __MMAP_WRITABLE)
#define MUNMAP(a, s)   (unmap_page((a), (s)), 0)
#define DIRECT_MMAP(s) MMAP(s)

/* silence unwanted warnings */
#pragma clang diagnostic ignored "-Wnull-pointer-arithmetic"
#pragma clang diagnostic ignored "-Wunused-variable"
