#include <kernel/mm/pmm.h>
#include <utils/memset.h>
#include <stddef.h>
#include <stdint.h>

extern char _end;
extern char _ram_end;
extern char _ram_start;

static uint8_t page_bitmap[MAX_PAGES / 8];

void pmm_init(void)
{
    memset(page_bitmap, 0, sizeof(page_bitmap));

    size_t kernel_pages = ((uintptr_t)&_end - (uintptr_t)&_ram_start + PAGE_SIZE - 1) / PAGE_SIZE;

    for (size_t i = 0; i < kernel_pages; i++)
    {
        size_t byte = i / 8;
        size_t bit  = i % 8;
        page_bitmap[byte] |= (1 << bit);
    }
}

void* pmm_alloc_page(void)
{
    for (size_t i = 0; i < MAX_PAGES; i++)
    {
        size_t byte = i / 8;
        size_t bit = i % 8;

        if (!(page_bitmap[byte] & (1 << bit)))
        {
            page_bitmap[byte] |= (1 << bit);

            uintptr_t addr = _ram_start + i * PAGE_SIZE;

            return (void*)addr;
        }
    }

    return NULL;
}

_Bool pmm_free_page(void* page)
{
    if ((uintptr_t)page < _ram_start) return 1;
    size_t page_index = ((uintptr_t)page - _ram_start) / PAGE_SIZE;
    if (page_index >= MAX_PAGES) return 1;
    size_t byte_index = page_index / 8;
    size_t bit_index  = page_index % 8;

    memset(page, 0, PAGE_SIZE);
    page_bitmap[byte_index] &= ~(1 << bit_index);
    return 0;
}