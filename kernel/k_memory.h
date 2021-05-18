#ifndef RENZAN_MEMORY_H_
#define RENZAN_MEMORY_H_

#ifdef _KERNEL

#include <k_mutex.h>
#include <k_atomic.h>

#include <osrt/types.h>

#define KMEM_PAGE_SIZE (2048 * 1024) /* 2MB */

struct address_space_s {
        atomic_uint ref_count;
        mutex_t     lock;
        u64         pml4_paddr;
};

typedef struct address_space_s address_space_t;

enum VM_PAGE_ATTR {
        VM_PAGE_PRESENT    = 1,
        VM_PAGE_WRITABLE   = 2,
        VM_PAGE_PRIVILEGED = 4,
        VM_PAGE_EXECUTABLE = 8,
        VM_PAGE_GLOBAL     = 256,
};

enum PMA_ZONES {
        PMA_ZONE_ANY,
};

#define USER_ALLOWED_FLAGS (VM_PAGE_WRITABLE | VM_PAGE_EXECUTABLE)

#define KERNEL_VADDR_BEGIN   ((void *)0xffffffff80000000)
#define KERNEL_VADDR_END     ((void *)0xffffffffffffffff)
#define USERLAND_VADDR_BEGIN ((void *)0x200000)
#define USERLAND_VADDR_END   ((void *)0x800000000000)

#define VM_ALIGN_PAGE_2M(vaddr)  ((vaddr) & ~0x1fffff)
#define VM_OFFSET_PAGE_2M(vaddr) ((vaddr)&0x1fffff)

void    pma_init(void);
uintptr pma_alloc_contiguous(enum PMA_ZONES zone, size_t size);
uintptr pma_alloc(enum PMA_ZONES zone);
void    pma_inc_ref_count(uintptr base, size_t size);
void    pma_dec_ref_count(uintptr base, size_t size);

void    vm_init(void);
boolean vm_is_mapped(address_space_t *as, void *vma);
void *  vm_map_page(address_space_t *as, void *vma, uintptr pma, uint attr);
uintptr vm_unmap_page(address_space_t *as, void *vma);
void    vm_unmap_n(address_space_t *as, void *vma, size_t size);

address_space_t *vm_address_space_create(void);
void             vm_address_space_destroy(address_space_t *as);
address_space_t *vm_address_space_clone(address_space_t *src);
void             vm_address_space_load(address_space_t *address_space);

uint  vm_check_attr(void *pml4_vaddr, void *vma, uint attr);
void *vm_alloc_vaddr(
    address_space_t *as, void *vaddr_begin, void *vaddr_end, size_t size);
void *vm_kern_map_page(void *vma, uintptr pma);
void *vm_kern_alloc_anywhere(size_t size);

void *  vm_get_vma_for_pma(uintptr pma);
uintptr vm_get_pma_by_vma(void *pml4_vaddr, void *vma);

void  kmem_heap_init(void);
void *kmem_alloc(size_t size);
void *kmem_alloc_zeroed(size_t size);
void *kmem_alloc_aligned(size_t size, size_t alignment);
void  kmem_free(void *ptr);
void *kmem_try_realloc(void *ptr, size_t new_size);
void *kmem_realloc(void *ptr, size_t new_size);

#else /* _KERNEL */

#define __PAGE_SIZE (4096 * 512)

enum MMAP_ATTR { __MMAP_WRITABLE = 2, __MMAP_EXECUTABLE = 8 };

#endif /* _KERNEL */

#endif /* RENZAN_MEMORY_H_ */
