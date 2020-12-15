#ifndef KAGUYA_USER_MEMORY_H__
#define KAGUYA_USER_MEMORY_H__

#include <os/kernel/memory.h>
#include <os/kernel/types.h>

boolean user_memory_check_read(address_space_t *as, void *vma, size_t size);
boolean user_memory_check_write(address_space_t *as, void *vma, size_t size);
boolean
user_memory_read(address_space_t *as, void *dst, void *src, size_t size);
boolean
user_memory_write(address_space_t *as, void *dst, void *src, size_t size);

/*
 * Copy memory from src_vaddr to dst_vaddr,
 * return value:
 *	-1 if dst do not have write permission at dst_vaddr,
 *	1 if src do not have read permission at src_vaddr,
 *	0 if success
 */
sint user_memory_copy(
    address_space_t *dst_as, void *dst_vaddr, address_space_t *src_as,
    void *src_vaddr, size_t size);

#endif /* KAGUYA_USER_MEMORY_H__ */
