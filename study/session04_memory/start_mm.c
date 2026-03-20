#include <vellum/mm.h>

#include <stddef.h>
#include <stdint.h>

/*
 * Starter source for session 04.
 * The solved overlay restores the real bitmap allocator and paging setup.
 */

static size_t pma_available_frames;
static size_t pma_free_frames;
static pfn_t pma_base_pfn;
static pfn_t pma_limit_pfn;

struct page_dir_recursive page_dir_recursive __aligned(PAGE_SIZE);
struct page_dir_recursive *_pc_page_dir;

status_t mm_pma_init(uintptr_t base_paddr, uintptr_t limit_paddr)
{
    /*
     * TODO(session 04-1):
     * Initialize the physical memory allocator state and reserve the bitmap.
     */
    if (limit_paddr < base_paddr) return STATUS_INVALID_VALUE;

    pma_base_pfn = base_paddr / PAGE_SIZE;
    pma_limit_pfn = limit_paddr / PAGE_SIZE;
    pma_available_frames = pma_limit_pfn - pma_base_pfn + 1;
    pma_free_frames = pma_available_frames;
    return STATUS_SUCCESS;
}

status_t mm_pma_mark_reserved(uintptr_t base_paddr, uintptr_t limit_paddr)
{
    /*
     * TODO(session 04-2):
     * Mark the PFN range as reserved in the bitmap and update the counters.
     */
    (void)base_paddr;
    (void)limit_paddr;
    return STATUS_NOT_IMPLEMENTED;
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
     * Find `count` contiguous free frames and return the base PFN.
     */
    (void)count;
    (void)pfn;
    return STATUS_NOT_IMPLEMENTED;
}

void mm_pma_free_frame(pfn_t pfn, size_t frame_count)
{
    (void)pfn;
    (void)frame_count;
}

status_t mm_init(void)
{
    /*
     * TODO(session 04-4):
     * Build the first page directory, load CR3, and enable paging.
     */
    _pc_page_dir = &page_dir_recursive;
    return STATUS_NOT_IMPLEMENTED;
}
