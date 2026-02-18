#include <kernel/mm/kmalloc.h>
#include <kernel/mm/pmm.h>
#include <kernel/kprint.h>
#include <kernel/kpanic.h>

static struct kmem_block *free_list = NULL;
static uintptr_t heap_start = 0;
static uintptr_t heap_end = 0;

static size_t align_up(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

void kmalloc_init(void) {
    heap_start = (uintptr_t)pmm_alloc_page();
    if (heap_start == 0) kpanic("failed to initiate kmalloc");
    heap_end  = heap_start + HEAP_PAGE_SIZE;

    free_list = (struct kmem_block*)heap_start;
    free_list->size = HEAP_PAGE_SIZE - sizeof(struct kmem_block);
    free_list->next = NULL;
}

static _Bool expand_heap(void) {
    struct kmem_block *block = (struct kmem_block*)pmm_alloc_page();
    if (block == NULL) return 1;

    block->size = HEAP_PAGE_SIZE - sizeof(struct kmem_block);
    block->next = free_list;
    free_list = block;
    heap_end += HEAP_PAGE_SIZE;
    return 0;
}

void *kmalloc(size_t size) {
    size = align_up(size);
    struct kmem_block **prev = &free_list;
    struct kmem_block *curr = free_list;

    while (curr) {
        if (curr->size >= size) {
            if (curr->size > size + sizeof(struct kmem_block)) {
                struct kmem_block *new_block = (struct kmem_block*)((uintptr_t)(curr + 1) + size);
                new_block->size = curr->size - size - sizeof(struct kmem_block);
                new_block->next = curr->next;

                curr->size = size;
                curr->next = NULL;
                *prev = new_block;
            } else {
                *prev = curr->next;
                curr->next = NULL;
            }
            return (void*)(curr + 1);
        }
        prev = &curr->next;
        curr = curr->next;
    }

    if (!expand_heap()) kpanic("kmalloc: out of memory");
    return kmalloc(size);
}

_Bool kfree(void *ptr) {
    if (!ptr) return 1;

    struct kmem_block *block = (struct kmem_block*)ptr - 1;
    block->next = free_list;
    free_list = block;
    return 0;
}