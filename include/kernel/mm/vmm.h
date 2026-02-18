#pragma once
#include <stdint.h>

extern uint32_t* kroot_table;

enum {
    VMM_PRESENT = 1 << 0,
    VMM_READ    = 1 << 1,
    VMM_WRITE   = 1 << 2,
    VMM_EXEC    = 1 << 3,
    VMM_USER    = 1 << 4
};

enum {
    PTE_V = 1 << 0,
    PTE_R = 1 << 1,
    PTE_W = 1 << 2,
    PTE_X = 1 << 3
};

void vmm_init(void);
_Bool vmm_unmap(uintptr_t virt);
_Bool vmm_map(uintptr_t virt, uintptr_t phys, uint32_t flags);
void* vmm_virt_to_phys(uintptr_t virt);
void* vmm_alloc_page(uint32_t flags);
_Bool  vmm_free_page(void* virt);