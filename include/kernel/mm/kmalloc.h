#pragma once
#include <stddef.h>
#include <stdint.h>

#define HEAP_PAGE_SIZE 4096
#define ALIGNMENT 8

struct kmem_block {
    size_t size;
    struct kmem_block *next;
};

void kmalloc_init(void);
void *kmalloc(size_t size);
_Bool kfree(void *ptr);