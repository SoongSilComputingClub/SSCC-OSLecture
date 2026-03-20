#include <vellum/mm.h>

#include <stddef.h>
#include <stdint.h>

#include <vellum/arch/intrinsics/register.h>
#include <vellum/macros.h>

/*
 * Skeleton source for session 04.
 * Reference implementation:
 * vellum/mm/mm.c
 */

#define PBM_GET(idx) ((pma_bitmap[(idx) >> 2] >> (((idx) & 3) * 2)) & 3)
#define PBM_SET(idx, val)                                                                          \
    do {                                                                                           \
        pma_bitmap[(idx) >> 2] &= ~(3 << (((idx) & 3) * 2));                                       \
        pma_bitmap[(idx) >> 2] |= (val) << (((idx) & 3) * 2);                                      \
    } while (0)

#define PBM_FREE      0
#define PBM_ALLOCATED 1
#define PBM_RESERVED  2

extern int __end;

static uint8_t *pma_bitmap;
static size_t pma_bitmap_size;
static size_t pma_frame_desc_count;
static size_t pma_available_frames, pma_free_frames;
static pfn_t pma_base_pfn, pma_limit_pfn;
static union VlA_PageTableEntry low_identity_pt[1024] __aligned(PAGE_SIZE);

struct page_dir_recursive page_dir_recursive __aligned(PAGE_SIZE);
struct page_dir_recursive *_pc_page_dir;

static void zero_bytes(void *buf, size_t len)
{
    size_t i;

    for (i = 0; i < len; ++i) {
        ((uint8_t *)buf)[i] = 0;
    }
}

static size_t pfn_to_bitmap_index(pfn_t pfn)
{
    return (size_t)(pfn - pma_base_pfn);
}

status_t mm_pma_init(uintptr_t base_paddr, uintptr_t limit_paddr)
{
    uintptr_t bitmap_start;
    uintptr_t bitmap_end;

    /*
     * TODO(session 04-1):
     * - Place the bitmap after `__end`
     * - Compute frame counts and PFN range
     * - Clear the bitmap
     * - Mark the bitmap storage itself as reserved
     */
    if (limit_paddr < base_paddr) return STATUS_INVALID_VALUE;

    pma_base_pfn = base_paddr / PAGE_SIZE;
    pma_limit_pfn = limit_paddr / PAGE_SIZE;
    pma_frame_desc_count = pma_limit_pfn - pma_base_pfn + 1;
    pma_bitmap_size = ALIGN_DIV(pma_frame_desc_count, 4);
    pma_available_frames = pma_frame_desc_count;
    pma_free_frames = pma_frame_desc_count;

    bitmap_start = ALIGN((uintptr_t)&__end, 4);
    bitmap_end = bitmap_start + pma_bitmap_size;
    pma_bitmap = (uint8_t *)bitmap_start;
    zero_bytes(pma_bitmap, pma_bitmap_size);

    /*
     * Keep the kernel image, page tables, and the bitmap itself out of the
     * allocator so later sessions do not hand those frames back to liballoc.
     */
    return mm_pma_mark_reserved(base_paddr, bitmap_end - 1);
}

status_t mm_pma_mark_reserved(uintptr_t base_paddr, uintptr_t limit_paddr)
{
    pfn_t start_pfn;
    pfn_t end_pfn;
    pfn_t current;

    /*
     * TODO(session 04-2):
     * Convert the physical address range to PFN range and mark each frame as
     * reserved while updating the accounting counters.
     */
    if (limit_paddr < base_paddr) return STATUS_INVALID_VALUE;

    start_pfn = base_paddr / PAGE_SIZE;
    end_pfn = limit_paddr / PAGE_SIZE;

    if (start_pfn < pma_base_pfn) {
        start_pfn = pma_base_pfn;
    }
    if (end_pfn > pma_limit_pfn) {
        end_pfn = pma_limit_pfn;
    }

    for (current = start_pfn; current <= end_pfn; ++current) {
        size_t index = pfn_to_bitmap_index(current);
        uint8_t state = PBM_GET(index);

        if (state == PBM_RESERVED) continue;
        if (state == PBM_FREE && pma_free_frames) {
            --pma_free_frames;
        }

        PBM_SET(index, PBM_RESERVED);
    }

    return STATUS_SUCCESS;
}

status_t mm_pma_get_available_frame_count(size_t *frame_count)
{
    if (frame_count) {
        *frame_count = pma_available_frames;
    }

    return STATUS_SUCCESS;
}

status_t mm_pma_get_free_frame_count(size_t *frame_count)
{
    if (frame_count) {
        *frame_count = pma_free_frames;
    }

    return STATUS_SUCCESS;
}

status_t mm_pma_allocate_frame(size_t count, pfn_t *pfn)
{
    /*
     * TODO(session 04-3):
     * Find `count` contiguous free frames, mark them allocated, and return the
     * base PFN.
     */
    size_t run = 0;
    size_t start = 0;
    size_t i;

    if (!count || !pfn) return STATUS_INVALID_VALUE;

    for (i = 0; i < pma_frame_desc_count; ++i) {
        if (PBM_GET(i) == PBM_FREE) {
            if (!run) start = i;
            ++run;
            if (run == count) {
                size_t j;

                for (j = 0; j < count; ++j) {
                    PBM_SET(start + j, PBM_ALLOCATED);
                }
                pma_free_frames -= count;
                *pfn = pma_base_pfn + start;
                return STATUS_SUCCESS;
            }
        } else {
            run = 0;
        }
    }

    return STATUS_INSUFFICIENT_MEMORY;
}

void mm_pma_free_frame(pfn_t pfn, size_t frame_count)
{
    size_t i;
    size_t start;

    if (!frame_count) return;
    if (pfn < pma_base_pfn || pfn > pma_limit_pfn) return;

    start = pfn_to_bitmap_index(pfn);
    for (i = 0; i < frame_count && start + i < pma_frame_desc_count; ++i) {
        if (PBM_GET(start + i) == PBM_ALLOCATED) {
            PBM_SET(start + i, PBM_FREE);
            ++pma_free_frames;
        }
    }
}

static status_t init_page_directory(void)
{
    size_t i;

    /*
     * TODO(session 04-4):
     * - Initialize a recursive page directory
     * - Allocate one page table for the low 4 MiB identity map
     * - Make `_pc_page_dir` point at the recursive mapping window
     */
    zero_bytes(&page_dir_recursive, sizeof(page_dir_recursive));
    zero_bytes(low_identity_pt, sizeof(low_identity_pt));

    for (i = 0; i < ARRAY_SIZE(low_identity_pt); ++i) {
        low_identity_pt[i].raw = 0;
        low_identity_pt[i].base = i;
        low_identity_pt[i].p = 1;
        low_identity_pt[i].r_w = 1;
    }

    page_dir_recursive.pde[0].raw = 0;
    page_dir_recursive.pde[0].dir.base = ((uintptr_t)low_identity_pt) >> 12;
    page_dir_recursive.pde[0].dir.p = 1;
    page_dir_recursive.pde[0].dir.r_w = 1;

    page_dir_recursive.pte.raw = 0;
    page_dir_recursive.pte.base = ((uintptr_t)&page_dir_recursive) >> 12;
    page_dir_recursive.pte.p = 1;
    page_dir_recursive.pte.r_w = 1;

    _pc_page_dir = &page_dir_recursive;
    return STATUS_SUCCESS;
}

status_t mm_init(void)
{
    status_t status;
    uint32_t cr0;

    /*
     * TODO(session 04-5):
     * - Call `init_page_directory`
     * - Load CR3
     * - Set the PG bit in CR0
     */
    status = init_page_directory();
    if (!CHECK_SUCCESS(status)) return status;

    VlA_WriteCr3((uint32_t)(uintptr_t)&page_dir_recursive);
    cr0 = VlA_ReadCr0();
    VlA_WriteCr0(cr0 | CR0_PG);
    return STATUS_SUCCESS;
}

status_t mm_vpn_to_pfn(vpn_t vpn, pfn_t *pfn)
{
    if (pfn) {
        *pfn = vpn;
    }

    return STATUS_SUCCESS;
}

status_t mm_vaddr_to_paddr(void *vaddr, uintptr_t *paddr)
{
    if (paddr) {
        *paddr = (uintptr_t)vaddr;
    }

    return STATUS_SUCCESS;
}

status_t mm_map(pfn_t pfn, vpn_t vpn, size_t page_count, uint32_t flags)
{
    (void)pfn;
    (void)vpn;
    (void)page_count;
    (void)flags;
    return STATUS_NOT_IMPLEMENTED;
}

status_t mm_unmap(vpn_t vpn, size_t page_count)
{
    (void)vpn;
    (void)page_count;
    return STATUS_NOT_IMPLEMENTED;
}

status_t mm_allocate_pages(size_t page_count, vpn_t *vpn)
{
    pfn_t pfn;
    status_t status;

    status = mm_pma_allocate_frame(page_count, &pfn);
    if (!CHECK_SUCCESS(status)) return status;

    if (vpn) {
        *vpn = pfn;
    }

    return STATUS_SUCCESS;
}

status_t mm_allocate_pages_to(vpn_t vpn, size_t page_count)
{
    (void)vpn;
    (void)page_count;
    return STATUS_NOT_IMPLEMENTED;
}

void mm_free_pages(vpn_t vpn, size_t page_count)
{
    mm_pma_free_frame(vpn, page_count);
}
