/* k_user_mem.c -- Safe userland memory access */
/* XXX should move to HAL */

/*
 * Copyright 2021 Mosakuji Hokuto <shikieiki@yamaxanadu.org>.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <k_console.h>
#include <k_string.h>
#include <k_user_mem.h>

/* lock address space before invoke */
boolean
user_memory_check_read(address_space_t *as, void *vma, size_t size)
{
        for (byte *p = vma; p < (byte *)vma + size; p += KMEM_PAGE_SIZE) {
                uint attr = vm_check_attr(
                    vm_get_vma_for_pma(as->pml4_paddr), p,
                    VM_PAGE_PRESENT | VM_PAGE_PRIVILEGED);

                if (attr != VM_PAGE_PRESENT) return B_FALSE;
        }

        return B_TRUE;
}

boolean
user_memory_check_write(address_space_t *as, void *vma, size_t size)
{
        for (byte *p = vma; p < (byte *)vma + size; p += KMEM_PAGE_SIZE) {
                uint attr = vm_check_attr(
                    vm_get_vma_for_pma(as->pml4_paddr), p,
                    VM_PAGE_PRESENT | VM_PAGE_WRITABLE | VM_PAGE_PRIVILEGED);

                if (attr != (VM_PAGE_PRESENT | VM_PAGE_WRITABLE)) {
                        return B_FALSE;
                }
        }

        return B_TRUE;
}

boolean
user_memory_read(address_space_t *as, void *dst, void *src, size_t size)
{
        mutex_acquire(&as->lock);

        if (!user_memory_check_read(as, src, size)) {
                mutex_release(&as->lock);
                return B_FALSE;
        }

        uintptr src_pma =
            vm_get_pma_by_vma(vm_get_vma_for_pma(as->pml4_paddr), src);
        src = vm_get_vma_for_pma(src_pma);
        kmemcpy(dst, src, size);

        mutex_release(&as->lock);
        return B_TRUE;
}

boolean
user_memory_write(address_space_t *as, void *dst, void *src, size_t size)
{
        mutex_acquire(&as->lock);

        if (!user_memory_check_write(as, dst, size)) {
                mutex_release(&as->lock);
                return B_FALSE;
        }

        uintptr dst_pma =
            vm_get_pma_by_vma(vm_get_vma_for_pma(as->pml4_paddr), dst);
        dst = vm_get_vma_for_pma(dst_pma);
        kmemcpy(dst, src, size);

        mutex_release(&as->lock);
        return B_TRUE;
}

sint
user_memory_copy(
    address_space_t *dst_as, void *dst_vaddr, address_space_t *src_as,
    void *src_vaddr, size_t size)
{
        mutex_acquire_dual(&src_as->lock, &dst_as->lock);

        if (!user_memory_check_read(src_as, src_vaddr, size)) {
                mutex_release_dual(&src_as->lock, &dst_as->lock);
                return 1;
        }

        if (!user_memory_check_write(dst_as, dst_vaddr, size)) {
                mutex_release_dual(&src_as->lock, &dst_as->lock);
                return -1;
        }

        uintptr src_pma = vm_get_pma_by_vma(
            vm_get_vma_for_pma(src_as->pml4_paddr), src_vaddr);
        void *src = vm_get_vma_for_pma(src_pma);

        uintptr dst_pma = vm_get_pma_by_vma(
            vm_get_vma_for_pma(dst_as->pml4_paddr), dst_vaddr);
        void *dst = vm_get_vma_for_pma(dst_pma);

        kmemmov(dst, src, size);

        mutex_release_dual(&src_as->lock, &dst_as->lock);
        return 0;
}
