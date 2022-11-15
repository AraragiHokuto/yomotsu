/* XXX to be reimplemented */

#include <hal_percpu.h>
#include <k_atomic.h>
#include <k_cdefs.h>
#include <k_console.h>
#include <k_memory.h>
#include <k_mutex.h>
#include <k_string.h>

#define ALIGNDOWN(x, r) ((x) & ~((r)-1))
#define ALIGNUP(x, r)   (ALIGNDOWN((x)-1, r) + (r))
#define DIV_CEIL(a, b)  ((((a)-1) / (b)) + 1)

/* === Physical Memory Allocation === */
/* TODO: implement a better allocator */

/* 4MB long map, enough for 16TB physical memory */

#define PM_MAP_ELEMENTS (4096 * 1024 / sizeof(u8)) /* 4MB */

static atomic_u8 pm_map[PM_MAP_ELEMENTS];

static mutex_t pm_map_lock;

static uintptr pm_upper_limit;

void
pma_init(void)
{
        mutex_init(&pm_map_lock);
        kmemset(pm_map, 0xff, sizeof(pm_map));
}

/*
 * Mark a region usable
 * Should only be called during bootup
 */
void
_pma_mark_usable(uintptr base, size_t size)
{
        if (base + size > pm_upper_limit) {
                pm_upper_limit = base + size;
        }
        /* scale base and size into page index */
        base /= KMEM_PAGE_SIZE;
        size /= KMEM_PAGE_SIZE;

        kmemset(&pm_map[base], 0, size);
}

/*
 * Mark a region reserved
 * Should only be called during bootup
 */
void
_pma_mark_reserved(uintptr base, size_t size)
{
        if (base + size > pm_upper_limit) {
                pm_upper_limit = base + size;
        }
        /* scale base and size into page index */
        base /= KMEM_PAGE_SIZE;
        size = DIV_CEIL(size, KMEM_PAGE_SIZE);

        kmemset(&pm_map[base], 0xff, size);
}

/* Increase the reference count in a range */
void
pma_inc_ref_count(uintptr base, size_t size)
{
        base /= KMEM_PAGE_SIZE;
        size /= KMEM_PAGE_SIZE;

        for (size_t i = base; i < base + size; ++i) {
                atomic_inc_fetch_u8(pm_map[i], __ATOMIC_ACQ_REL);
        }
}

/* Decrease the reference count in a range */
void
pma_dec_ref_count(uintptr base, size_t size)
{
        base /= KMEM_PAGE_SIZE;
        size /= KMEM_PAGE_SIZE;

        for (size_t i = base; i < base + size; ++i) {
                atomic_dec_fetch_u8(pm_map[i], __ATOMIC_ACQ_REL);
        }
}

/* Allocate size bytes of contiguous physical page */
uintptr
pma_alloc_contiguous(enum PMA_ZONES zone, size_t size)
{
        /* zone not implemented yet */
        DONTCARE(zone);

        uint page_count = DIV_CEIL(size, KMEM_PAGE_SIZE);

        /* don't support that much memory */
        if (page_count > PM_MAP_ELEMENTS) return 0;

        mutex_acquire(&pm_map_lock);

        for (uint i = 0; i < PM_MAP_ELEMENTS - page_count; ++i) {
                if (pm_map[i]) continue;

                boolean valid = B_TRUE;
                for (uint j = i; j < (i + page_count); ++j) {
                        if (pm_map[j]) {
                                valid = B_FALSE;
                                break;
                        }
                }
                if (valid) {
                        /* mark range as used */
                        kmemset(&pm_map[i], 1, page_count);

                        mutex_release(&pm_map_lock);

                        uintptr ret = i * KMEM_PAGE_SIZE;
                        return ret;
                }
        }

        /* OOM */
        mutex_release(&pm_map_lock);
        return 0;
}

/* Allocate one page from physical memory */
static uintptr
_pma_alloc_locked(enum PMA_ZONES zone)
{
        /* XXX: zone not implemented yet */
        DONTCARE(zone);

        mutex_acquire(&pm_map_lock);

        /* find a free page */
        for (uint i = 0; i < PM_MAP_ELEMENTS; ++i) {
                if (pm_map[i]) continue;

                ++pm_map[i];
                return i * KMEM_PAGE_SIZE;
        }

        /* OOM */
        mutex_release(&pm_map_lock);
        return 0;
}

uintptr
pma_alloc(enum PMA_ZONES zone)
{
        /* find a free page */
        for (uint i = 0; i < PM_MAP_ELEMENTS; ++i) {
                if (pm_map[i]) continue;

                if (atomic_inc_fetch_u8(pm_map[i], __ATOMIC_ACQ_REL) != 1) {
                        /*
                         * this frame was allocated somewhere inbetween,
                         * rollback
                         */
                        atomic_dec_fetch_u8(pm_map[i], __ATOMIC_ACQ_REL);
                }

                return i * KMEM_PAGE_SIZE;
        }

        /*
         * alloc failed, try locked version
         * to make sure we didn't miss anything
         */
        return _pma_alloc_locked(zone);
}

void
pma_free(uintptr base, size_t size)
{
        pma_dec_ref_count(base, size);
}

/* === Paging === */

/* PML4 entry on AMD64 */
typedef struct pml4e_s {
        u64 present : 1, rw : 1, privileged : 1, cache : 2, accessed : 1,
            mbz : 3, avl : 3, addr : 40, avaliable : 10, nx : 1;
} PACKED pml4e_t;

/* PDP entry on AMD64 */
typedef struct pdpe_s {
        u64 present : 1, rw : 1, privileged : 1, cache : 2, accessed : 1,
            dirty : 1, page_size : 1, global : 1, avl : 3, addr : 40,
            available : 10, nx : 1;
} PACKED pdpe_t;

/* PD entry on AMD64 */
typedef struct pde_s {
        u64 present : 1, rw : 1, privileged : 1, cache : 2, accessed : 1,
            dirty : 1, page_size : 1, global : 1, avl : 2, os_no_return : 1,
            addr : 40, available : 10, nx : 1;
} PACKED pde_t;

/*
 * Our current implementation uses recursive mapping
 * with the 256th entry of PML4 mapped back to itself
 */

static pml4e_t *
get_pml4e(address_space_t *as, uint pml4_idx)
{
        pml4e_t *pml4 = as ? vm_get_vma_for_pma(as->pml4_paddr)
                           : (void *)0xFFFF804020100000;

        return &pml4[pml4_idx];
}

static pdpe_t *
get_pdpe(pml4e_t *pml4e, uint pdp_idx)
{
        pdpe_t *pdp = vm_get_vma_for_pma(pml4e->addr << 12);

        return &pdp[pdp_idx];
}

static pde_t *
get_pde(pdpe_t *pdpe, uint pd_idx)
{
        pde_t *pd = vm_get_vma_for_pma(pdpe->addr << 12);

        return &pd[pd_idx];
}

static void *
sign_extend(void *address)
{
        u64 extended = ((u64)address & (1ull << 47) ? ~(0ull) : 0);
        return (
            void
                *)((extended & 0xffff000000000000) | ((u64)address & 0xffffffffffff));
}

static void *
get_page(u64 pml4_idx, u64 pdp_idx, u64 pd_idx)
{
        return sign_extend(
            (void *)((pml4_idx << 39) | (pdp_idx << 30) | (pd_idx << 21)));
}

#define PML4IDX(va) ((((uintptr)va) >> 39) & 511)
#define PDPIDX(va)  ((((uintptr)va) >> 30) & 511)
#define PDIDX(va)   ((((uintptr)va) >> 21) & 511)

static mutex_t vm_kern_table_lock;

static pde_t *
get_pde_by_vma(void *pml4_vaddr, void *vma)
{
        pml4e_t *pml4e = pml4_vaddr;
        pml4e += PML4IDX(vma);
        if (!pml4e->present) return NULL;

        pdpe_t *pdpe = vm_get_vma_for_pma(pml4e->addr << 12);
        pdpe += PDPIDX(vma);
        if (!pdpe->present) return NULL;

        pde_t *pde = vm_get_vma_for_pma(pdpe->addr << 12);
        pde += PDIDX(vma);

        return pde;
}

uintptr
vm_get_pma_by_vma(void *pml4_vaddr, void *vma)
{
        pde_t *pde = get_pde_by_vma(pml4_vaddr, vma);
        return (pde && pde->present)
                   ? ((pde->addr << 12) + VM_OFFSET_PAGE_2M((uintptr)vma))
                   : 0;
}

static uint
get_attrs_by_vma(pde_t *pde)
{
        uint ret = 0;

        if (pde->present) {
                ret |= VM_PAGE_PRESENT;
        }

        if (pde->privileged == 0) {
                ret |= VM_PAGE_PRIVILEGED;
        }

        if (pde->rw) {
                ret |= VM_PAGE_WRITABLE;
        }

        if (!pde->nx) {
                ret |= VM_PAGE_EXECUTABLE;
        }

        if (pde->global) {
                ret |= VM_PAGE_GLOBAL;
        }

        return ret;
}

uint
vm_check_attr(void *pml4_vaddr, void *vma, uint attr)
{
        return attr & get_attrs_by_vma(get_pde_by_vma(pml4_vaddr, vma));
}

void *
vm_alloc_vaddr(
    address_space_t *as, void *vaddr_begin, void *vaddr_end, size_t size)
{
        byte *p = vaddr_begin;
        while (p < (byte *)vaddr_end - size) {
                if (vm_is_mapped(as, p)) {
                        p += KMEM_PAGE_SIZE;
                        continue;
                }

                boolean valid = B_TRUE;
                byte   *q;
                for (q = p; q < p + size; q += KMEM_PAGE_SIZE) {
                        if (vm_is_mapped(as, q)) {
                                valid = B_FALSE;
                                break;
                        }
                }

                if (valid) {
                        return p;
                } else {
                        p = q;
                }
        }

        return NULL;
}

static uintptr
alloc_pt()
{
        /*
         * XXX: by allocating a 2MB page every time, we
         * are wasting TONS of memory right now,
         * but I'm too lazy so I'll fix that later.
         */

        return pma_alloc(PMA_ZONE_ANY);
}

static void
free_pt(uintptr pt)
{
        return pma_free(pt, KMEM_PAGE_SIZE);
}

static u64
swap_cr3(u64 value)
{
        uint ret;
        asm volatile(
            "movq %%cr3, %0\n"
            "movq %1, %%cr3\n"
            : "=r"(ret)
            : "r"(value));
        return ret;
}

static void
reload_cr3(void)
{
        u64 cr3;
        asm volatile(
            "movq %%cr3, %0\n"
            "movq %0, %%cr3\n"
            : "=r"(cr3)
            : "r"(cr3));
}

/*
 * Map one page from VMA to PMA
 * The caller must ensure that PMA is avaliable
 */
void *
vm_map_page(address_space_t *as, void *vma, uintptr pma, uint attr)
{
        pml4e_t *pml4e = get_pml4e(as, (PML4IDX(vma)));
        if (!pml4e->present) {
                uintptr pdp_pa = alloc_pt();
                if (pdp_pa == 0) return NULL;

                void *pdp_va = vm_get_vma_for_pma(pdp_pa);
                kmemset(pdp_va, 0, KMEM_PAGE_SIZE);

                pml4e->rw      = 1;
                pml4e->present = 1;
                pml4e->addr    = pdp_pa >> 12;
        }

        pdpe_t *pdpe = get_pdpe(pml4e, PDPIDX(vma));
        if (!pdpe->present) {
                uintptr pd_pa = alloc_pt();
                if (pd_pa == 0) return NULL;

                void *pd_va = vm_get_vma_for_pma(pd_pa);
                kmemset(pd_va, 0, KMEM_PAGE_SIZE);

                pdpe->rw      = 1;
                pdpe->present = 1;
                pdpe->addr    = pd_pa >> 12;
        }

        pde_t *pde = get_pde(pdpe, PDIDX(vma));

        if (pde->present) {
                if (pma >> 12 == pde->addr) {
                        return vma;
                } else {
                        PANIC("remapping a mapped page");
                }
        }

        pml4e->privileged |= (attr >> 2) ^ 1;
        pml4e->rw |= (attr >> 1) & 1;
        pml4e->nx &= (attr >> 3) ^ 1;

        pdpe->privileged = (attr >> 2) ^ 1;
        pdpe->rw         = (attr >> 1) & 1;
        pdpe->nx         = (attr >> 3) ^ 1;

        pde->present    = 1;
        pde->privileged = (attr >> 2) ^ 1;
        pde->page_size  = 1;
        pde->rw         = (attr >> 1) & 1;
        pde->nx         = (attr >> 3) ^ 1;
        pde->global     = (attr >> 8) & 1;
        pde->addr       = pma >> 12;

        return vma;
}

boolean
vm_is_mapped(address_space_t *as, void *vma)
{
        pml4e_t *pml4e = get_pml4e(as, PML4IDX(vma));
        if (!pml4e->present) return B_FALSE;

        pdpe_t *pdpe = get_pdpe(pml4e, PDPIDX(vma));
        if (!pdpe->present) return B_FALSE;

        pde_t *pde = get_pde(pdpe, PDIDX(vma));
        return !!pde->present;
}

static void
vm_empty_pd(pde_t *pd)
{
        for (size_t i = 0; i < 512; ++i) {
                if (!pd[i].present) continue;
                pma_free(pd[i].addr << 12, KMEM_PAGE_SIZE);
                pd[i].present = 0;
        }
}

static void
vm_empty_pdp(pdpe_t *pdp)
{
        for (size_t i = 0; i < 512; ++i) {
                if (!pdp[i].present) continue;
                pde_t *pd = vm_get_vma_for_pma(pdp[i].addr << 12);
                vm_empty_pd(pd);
                free_pt(pdp[i].addr << 12);
                pdp[i].present = 0;
        }
}

static void
vm_empty_userland(pml4e_t *pml4)
{
        for (size_t i = 0; i < 256; ++i) {
                if (!pml4[i].present) continue;
                pdpe_t *pdp = vm_get_vma_for_pma(pml4[i].addr << 12);
                vm_empty_pdp(pdp);
                pml4[i].present = 0;
                free_pt(pml4[i].addr << 12);
        }
}

static uintptr
vm_clone_pd(uintptr pd_paddr)
{
        uintptr ret = alloc_pt();
        if (!ret) return 0;

        pde_t *src_pd = vm_get_vma_for_pma(pd_paddr);
        pde_t *dst_pd = vm_get_vma_for_pma(ret);
        if (!dst_pd) {
                free_pt(ret);
                return 0;
        }

        kmemcpy(dst_pd, src_pd, 4096);

        for (size_t i = 0; i < 512; ++i) {
                if (!src_pd[i].present) {
                        continue;
                }

                uintptr pm = pma_alloc(PMA_ZONE_ANY);
                if (!pm) {
                        /*
                         * return 0 to indicate failure.
                         * no need to rollback here since the whole
                         * address space will be destroyed
                         * in vm_clone_pml4().
                         */
                        return 0;
                }
                /* TODO: copy on write */
                void *src_page = vm_get_vma_for_pma(src_pd[i].addr << 12);
                void *dst_page = vm_get_vma_for_pma(pm);

                kmemcpy(dst_page, src_page, KMEM_PAGE_SIZE);

                dst_pd[i].addr = pm >> 12;
        }

        return ret;
}

static uintptr
vm_clone_pdp(uintptr pdp_paddr)
{
        uintptr ret = alloc_pt();
        if (!ret) return 0;

        pdpe_t *src_pdp = vm_get_vma_for_pma(pdp_paddr);
        pdpe_t *dst_pdp = vm_get_vma_for_pma(ret);
        if (!dst_pdp) {
                free_pt(ret);
                return 0;
        }

        kmemcpy(dst_pdp, src_pdp, 4096);

        for (size_t i = 0; i < 512; ++i) {
                if (!src_pdp[i].present) {
                        continue;
                }

                uintptr pm = vm_clone_pd(src_pdp[i].addr << 12);
                if (!pm) {
                        /*
                         * return 0 to indicate failure.
                         * no need to rollback here since the whole
                         * address space will be destroyed
                         * in vm_clone_pml4().
                         */
                        return 0;
                }
                dst_pdp[i].addr = pm >> 12;
        }

        return ret;
}

static uintptr
vm_clone_pml4(uintptr src_paddr, uintptr dst_paddr)
{
        pml4e_t *src_pml4 = vm_get_vma_for_pma(src_paddr);
        pml4e_t *dst_pml4 = vm_get_vma_for_pma(dst_paddr);
        if (!dst_pml4) {
                free_pt(dst_paddr);
                return 0;
        }

        kmemcpy(dst_pml4, src_pml4, 4096);

        /*
         * notice that we only copy userland space
         * -- kernel space is shared among all processes
         */
        for (size_t i = 0; i < 256; ++i) {
                if (!src_pml4[i].present) {
                        continue;
                }

                uintptr pm = vm_clone_pdp(src_pml4[i].addr << 12);
                if (!pm) {
                        for (size_t j = 0; j < i; ++j) {
                                pdpe_t *pdp =
                                    vm_get_vma_for_pma(dst_pml4[i].addr << 12);
                                vm_empty_pdp(pdp);
                                free_pt(dst_pml4[j].addr << 12);
                        }

                        return 0;
                }

                dst_pml4[i].addr = pm >> 12;
        }

        return dst_paddr;
}

address_space_t *
vm_address_space_clone(address_space_t *src)
{
        address_space_t *ret = vm_address_space_create();
        if (!ret) return NULL;

        if (!vm_clone_pml4(src->pml4_paddr, ret->pml4_paddr)) {
                vm_address_space_destroy(ret);
                return NULL;
        }

        return ret;
}

void
vm_address_space_load(address_space_t *as)
{
        asm volatile("movq %0, %%cr3" ::"r"(as->pml4_paddr));
}

address_space_t *
vm_address_space_create(void)
{
        uintptr pm = alloc_pt();
        if (!pm) return NULL;

        pml4e_t *pml4 = vm_get_vma_for_pma(pm);
        if (!pml4) {
                free_pt(pm);
                return NULL;
        }

        pml4e_t *current = get_pml4e(NULL, 0);
        kmemset(pml4, 0, 256 * sizeof(pml4e_t));
        kmemcpy(&pml4[256], &current[256], 256 * sizeof(pml4e_t));

        /* every pml4's 256th entry is mapped to itself */
        pml4[256].addr = pm >> 12;

        address_space_t *ret = kmem_alloc(sizeof(address_space_t));
        if (!ret) {
                free_pt(pm);
                return NULL;
        }

        ret->ref_count = 0;
        mutex_init(&ret->lock);
        ret->pml4_paddr = pm;

        return ret;
}

void
vm_address_space_destroy(address_space_t *ptr)
{
        if (ptr->pml4_paddr) {
                vm_empty_userland(vm_get_vma_for_pma(ptr->pml4_paddr));
        }
        kmem_free(ptr);
}

extern void _pml4;

#define PM_MAP_BASE ((byte *)0xffff808000000000)

#define RECURSIVE_PML4(va) ((pml4e_t *)0xffff804020100000)
#define RECURSIVE_PDP(va)  ((pdpe_t *)(0xffff804020000000 | (PML4IDX(va) << 12)))
#define RECURSIVE_PD(va) \
        ((pde_t          \
              *)(0xffff804000000000 | ((PML4IDX(va) << 21) | (PDPIDX(va) << 12))))

void
vm_init(void)
{
        mutex_init(&vm_kern_table_lock);

        /* map all physical memories into kernel space */
        for (uintptr p = 0; p < pm_upper_limit; p += KMEM_PAGE_SIZE) {
                void *va = p + PM_MAP_BASE;

                /* pml4e by recursive mapping */
                pml4e_t *pml4e = RECURSIVE_PML4(va) + PML4IDX(va);

                if (!pml4e->present) {
                        uintptr pt = alloc_pt();
                        ASSERT(pt);
                        pml4e->present = 1;
                        pml4e->addr    = pt >> 12;
                        kmemset(RECURSIVE_PDP(va), 0, 4096);
                }

                pdpe_t *pdpe = RECURSIVE_PDP(va) + PDPIDX(va);
                if (!pdpe->present) {
                        uintptr pt = alloc_pt();
                        ASSERT(pt);
                        pdpe->present = 1;
                        pdpe->addr    = pt >> 12;
                        kmemset(RECURSIVE_PD(va), 0, 4096);
                }

                pde_t *pd = RECURSIVE_PD(va) + PDIDX(va);
                ASSERT(!pd->present);
                pd->page_size = 1;
                pd->present   = 1;
                pd->global    = 1;
                pd->addr      = p >> 12;
        }
}

/*
 * Unmap one page by VMA
 * Return the PMA of which the page was mapped to
 */
uintptr
vm_unmap_page(address_space_t *as, void *_vma)
{
        uintptr  vma   = (uintptr)_vma;
        pml4e_t *pml4e = get_pml4e(as, PML4IDX(vma));
        VERIFY(pml4e->present, "unmapping an non-existing page");
        pdpe_t *pdpe = get_pdpe(pml4e, PDPIDX(vma));
        VERIFY(pdpe->present, "unmapping an non-existing page");
        pde_t *pde = get_pde(pdpe, PDIDX(vma));
        VERIFY(pde->present, "unmapping an non-existing page");

        uintptr ret = pde->addr << 12;
        kmemset(pde, 0, sizeof(pde_t));
        asm volatile("invlpg (%0)" ::"r"(_vma));

        return ret;
}

void
vm_unmap_n(address_space_t *as, void *_vma, size_t size)
{
        if (size == 0) return;

        byte *begin = _vma;
        byte *end   = begin + size;

        begin = (void *)ALIGNDOWN((uintptr)begin, KMEM_PAGE_SIZE);

        for (byte *p = begin; p < end; p += KMEM_PAGE_SIZE) {
                vm_unmap_page(as, p);
        }
}

void *
vm_kern_map_page(void *vma, uintptr pma)
{
        mutex_acquire(&vm_kern_table_lock);
        void *ret = vm_map_page(
            NULL, vma, pma,
            VM_PAGE_WRITABLE | VM_PAGE_EXECUTABLE | VM_PAGE_PRIVILEGED
                | VM_PAGE_GLOBAL);
        mutex_release(&vm_kern_table_lock);
        return ret;
}

void *
vm_get_vma_for_pma(uintptr pma)
{
        return pma + PM_MAP_BASE;
}

void *
vm_kern_alloc_anywhere(size_t size)
{
        uint page_count = DIV_CEIL(size, KMEM_PAGE_SIZE);

        mutex_acquire(&vm_kern_table_lock);
        byte *vma =
            vm_alloc_vaddr(NULL, KERNEL_VADDR_BEGIN, KERNEL_VADDR_END, size);
        if (!vma) {
                mutex_release(&vm_kern_table_lock);
                return NULL;
        }

        void *ret = vma;

        for (uint i = 0; i < page_count; ++i) {
                uintptr pma = pma_alloc(PMA_ZONE_ANY);
                if (!pma) goto rollback;

                if (!vm_map_page(
                        NULL, vma, pma,
                        VM_PAGE_WRITABLE | VM_PAGE_EXECUTABLE
                            | VM_PAGE_PRIVILEGED | VM_PAGE_GLOBAL))
                        goto rollback;

                vma += KMEM_PAGE_SIZE;
                pma += KMEM_PAGE_SIZE;

                continue;
rollback:
                for (byte *rp = ret; rp < vma; rp += KMEM_PAGE_SIZE) {
                        uintptr pma_unmapped = vm_unmap_page(NULL, rp);
                        pma_free(pma_unmapped, KMEM_PAGE_SIZE);
                }
                mutex_release(&vm_kern_table_lock);
                return NULL;
        }

        mutex_release(&vm_kern_table_lock);
        return ret;
}

/* === Kernel Heap === */

#define KHEAP_MAGIC_FREE      0xDEADBEEFDEADBEEF
#define KHEAP_MAGIC_ALLOCATED 0xDEADBABADEADBABA

/* must be 32 bytes long */
struct kheap_chunk_s {
        list_node_t chunk_list_node;
        u64         size;
        u64         magic;
};

typedef struct kheap_chunk_s kheap_chunk_t;

#define KHEAP_MAGIC_ALIGNED 0xBEEFBABADEADDEAD

struct kheap_aligned_header_s {
        void *free_ptr;
        u64   magic;
};

typedef struct kheap_aligned_header_s kheap_aligned_header_t;

static list_node_t chunk_list_head;

static mutex_t kheap_lock;

#define KHEAP_CHUNK_MIN_SIZE 32

void
kmem_heap_init(void)
{
        mutex_init(&kheap_lock);

        kheap_chunk_t *first_chunk = vm_alloc_vaddr(
            NULL, KERNEL_VADDR_BEGIN, KERNEL_VADDR_END, KMEM_PAGE_SIZE);
        uintptr first_chunk_pm = pma_alloc(PMA_ZONE_ANY);
        VERIFY(first_chunk, "failed to alloc vaddr for first chunk");
        VERIFY(first_chunk_pm, "failed to alloc paddr for first chunk");

        VERIFY(
            vm_kern_map_page(first_chunk, first_chunk_pm),
            "failed to map first chunk");

        list_head_init(&chunk_list_head);

        first_chunk->size  = KMEM_PAGE_SIZE - sizeof(kheap_chunk_t);
        first_chunk->magic = KHEAP_MAGIC_FREE;

        list_insert(&first_chunk->chunk_list_node, &chunk_list_head);
}

static boolean
kheap_chunk_is_free(kheap_chunk_t *chunk)
{
        VERIFY(
            chunk->magic == KHEAP_MAGIC_FREE
                || chunk->magic == KHEAP_MAGIC_ALLOCATED,
            "invalid chunk magic: %x at 0x%x", chunk->magic, chunk);

        return chunk->magic == KHEAP_MAGIC_FREE;
}

static boolean
kheap_expand(size_t min_expand_size)
{
        min_expand_size += sizeof(kheap_chunk_t);
        kheap_chunk_t *chunk = vm_kern_alloc_anywhere(min_expand_size);

        if (!chunk) return B_FALSE;

        chunk->size =
            ALIGNUP(min_expand_size, KMEM_PAGE_SIZE) - sizeof(kheap_chunk_t);
        chunk->magic = KHEAP_MAGIC_FREE;

        list_insert(&chunk->chunk_list_node, chunk_list_head.prev);

        return B_TRUE;
}

void *
kmem_alloc(size_t size)
{
        mutex_acquire(&kheap_lock);

        size = ALIGNUP(size, 32);

        kheap_chunk_t *p;
        list_node_t   *i;
        for (i = chunk_list_head.next; i != &chunk_list_head; i = i->next) {
                p = CONTAINER_OF(i, kheap_chunk_t, chunk_list_node);

                if (!kheap_chunk_is_free(p)) continue;

                if (p->size >= size) break;
        }

        if (i == &chunk_list_head) {
                /* no chunk avaliable for request, expand head */
                if (!kheap_expand(size)) {
                        /* OOM */
                        mutex_release(&kheap_lock);
                        return NULL;
                }

                p = CONTAINER_OF(
                    chunk_list_head.prev, kheap_chunk_t, chunk_list_node);
                ASSERT(p->size >= size);
        }

        if (p->size - size >= KHEAP_CHUNK_MIN_SIZE + sizeof(kheap_chunk_t)) {
                /* split chunk */
                byte *_new_chunk = (void *)p;
                _new_chunk += sizeof(kheap_chunk_t);
                _new_chunk += size;

                kheap_chunk_t *new_chunk = (void *)_new_chunk;
                new_chunk->size          = p->size - size - sizeof(*new_chunk);
                new_chunk->magic         = KHEAP_MAGIC_FREE;

                p->size = size;

                list_insert(&new_chunk->chunk_list_node, &p->chunk_list_node);
        }

        ASSERT(kheap_chunk_is_free(p));
        p->magic = KHEAP_MAGIC_ALLOCATED;

        mutex_release(&kheap_lock);
        void *ret = (void *)(((byte *)p) + sizeof(kheap_chunk_t));
        return ret;
}

void *
kmem_alloc_zeroed(size_t size)
{
        void *ret = kmem_alloc(size);
        if (!ret) return NULL;

        kmemset(ret, 0, size);
        return ret;
}

/* Align must be power of 2 */
void *
kmem_alloc_aligned(size_t size, size_t align)
{
        if (align < 32) {
                return kmem_alloc(size);
        }

        void *ptr = kmem_alloc(size + align);
        if (!ptr) return ptr;

        void *ret = (void *)ALIGNUP((u64)ptr, align);
        if (ret == ptr) {
                /* already aligned */
                return ret;
        }

        ASSERT((u64)ret - (u64)ptr < align);
        ASSERT((u64)ret - (u64)ptr >= sizeof(kheap_aligned_header_t));

        kheap_aligned_header_t *h =
            (void *)((byte *)ret - sizeof(kheap_aligned_header_t));

        h->free_ptr = ptr;
        h->magic    = KHEAP_MAGIC_ALIGNED;

        return ret;
}

static kheap_chunk_t *
kheap_merge_chunk_into_prev(kheap_chunk_t *chunk)
{
        kheap_chunk_t *prev = CONTAINER_OF(
            chunk->chunk_list_node.prev, kheap_chunk_t, chunk_list_node);

        list_remove(&chunk->chunk_list_node);
        prev->size += chunk->size;
        prev->size += sizeof(kheap_chunk_t);

        return prev;
}

static kheap_chunk_t *
kheap_chunk_from_ptr(void *ptr)
{
        return (void *)(((byte *)ptr) - sizeof(kheap_chunk_t));
}

void
kmem_free(void *ptr)
{
        mutex_acquire(&kheap_lock);

        kheap_aligned_header_t *alignh =
            (void *)((byte *)ptr - sizeof(kheap_aligned_header_t));

        if (alignh->magic == KHEAP_MAGIC_ALIGNED) {
                ptr = alignh->free_ptr;
        }

        kheap_chunk_t *chunk = kheap_chunk_from_ptr(ptr);
        VERIFY(
            chunk->magic == KHEAP_MAGIC_ALLOCATED,
            "invalid free: got magic %x for 0x%x", chunk->magic, chunk);

        /* mark chunk as free */
        chunk->magic = KHEAP_MAGIC_FREE;

        kheap_chunk_t *prev = chunk->chunk_list_node.prev == &chunk_list_head
                                  ? NULL
                                  : CONTAINER_OF(
                                      chunk->chunk_list_node.prev,
                                      kheap_chunk_t, chunk_list_node);
        kheap_chunk_t *next = chunk->chunk_list_node.next == &chunk_list_head
                                  ? NULL
                                  : CONTAINER_OF(
                                      chunk->chunk_list_node.next,
                                      kheap_chunk_t, chunk_list_node);

        /* merge with neighbors */
        if (prev && kheap_chunk_is_free(prev)) {
                chunk = kheap_merge_chunk_into_prev(chunk);
        }

        if (next && kheap_chunk_is_free(next)) {
                kheap_merge_chunk_into_prev(next);
        }

        mutex_release(&kheap_lock);
}

/* Attempt inplace realloc, return NULL if impossible */
void *
kmem_try_realloc(void *ptr, size_t new_size)
{
        /* stub */
        DONTCARE(ptr);
        DONTCARE(new_size);
        return NULL;
}

void *
kmem_realloc(void *ptr, size_t new_size)
{
        void *ret = kmem_try_realloc(ptr, new_size);
        if (ret) return ret;

        ret = kmem_alloc(new_size);
        if (ret == NULL) return NULL;

        VERIFY(ptr, "invalid ptr");

        size_t old_size = kheap_chunk_from_ptr(ptr)->size;
        kmemcpy(ret, ptr, old_size);
        kmem_free(ptr);
        return ret;
}
