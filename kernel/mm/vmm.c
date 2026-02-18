#include <kernel/mm/pmm.h>
#include <kernel/mm/vmm.h>
#include <kernel/kpanic.h>
#include <utils/memset.h>
#include <stdint.h>

uint32_t* kroot_table;

static inline void write_satp(uint32_t value)
{
    asm volatile("csrw satp, %0" :: "r"(value));
}

_Bool vmm_map(uintptr_t virt, uintptr_t phys, uint32_t flags)
{
    uint32_t vpn0 = (virt >> 12) & 0x3FF;
    uint32_t vpn1 = (virt >> 22) & 0x3FF;

    uint32_t* second;

    if (!(kroot_table[vpn1] & PTE_V)) {
        second = (uint32_t*)pmm_alloc_page();
        if (!second) return 1;

        memset(second, 0, PAGE_SIZE);

        uintptr_t second_phys = (uintptr_t)second;

        kroot_table[vpn1] = ((second_phys >> 12) << 10) | PTE_V;
    } else {
        second = (uint32_t*)((kroot_table[vpn1] >> 10) << 12);
    }

    second[vpn0] = ((phys >> 12) << 10) | flags | PTE_V;
    return 0;
}

_Bool vmm_unmap(uintptr_t virt)
{
    uint32_t vpn0 = (virt >> 12) & 0x3FF;
    uint32_t vpn1 = (virt >> 22) & 0x3FF;

    if (!(kroot_table[vpn1] & PTE_V)) return 1;

    uint32_t* second = (uint32_t*)((kroot_table[vpn1] >> 10) << 12);
    if (!(second[vpn0] & PTE_V)) return 1;

    second[vpn0] = 0;
    asm volatile("sfence.vma");
    return 0;
}

void* vmm_virt_to_phys(uintptr_t virt)
{
    uint32_t vpn1 = (virt >> 22) & 0x3FF;
    uint32_t vpn0 = (virt >> 12) & 0x3FF;
    uint32_t offset = virt & 0xFFF;

    if (!(kroot_table[vpn1] & PTE_V)) return 0;

    uint32_t* second = (uint32_t*)((kroot_table[vpn1] >> 10) << 12);
    if (!(second[vpn0] & PTE_V)) return NULL;

    uintptr_t phys_page = ((second[vpn0] >> 10) << 12);
    return (void*)(phys_page + offset);
}

void* vmm_alloc_page(uint32_t flags)
{
    void* phys = pmm_alloc_page();
    if (!phys) return NULL;

    uintptr_t addr = (uintptr_t)phys;

    if (!vmm_map(addr, addr, flags)) return NULL;

    return (void*)addr;
}

_Bool vmm_free_page(void* virt)
{
    uintptr_t vaddr = (uintptr_t)virt;

    uintptr_t phys = (uintptr_t)vmm_virt_to_phys(vaddr);
    if (!phys) return 1;

    if (!vmm_unmap(vaddr)) return 1;

    pmm_free_page((void*)phys);

    return 0;
}

void vmm_init(void)
{
    kroot_table = (uint32_t*)pmm_alloc_page();
    if (!kroot_table) kpanic("failed to initiate virtual memory management");
    memset(kroot_table, 0, PAGE_SIZE);

    uint32_t root_phys = (uint32_t)kroot_table;
    uint32_t satp_value = (1 << 31) | (root_phys >> 12);
    write_satp(satp_value);

    asm volatile("sfence.vma");
}