#include <kernel/virtio.h>
#include <kernel/kprint.h>
#include <kernel/pci.h>
#include <stdint.h>

volatile uint32_t* virtio_mmio = 0;

uint32_t virtio_read(uint32_t offset) {
    return virtio_mmio[offset / 4];
}

void virtio_write(uint32_t offset, uint32_t value) {
    virtio_mmio[offset / 4] = value;
}

void virtio_init() {
    pci_device dev = pci_scan_virtio_keyboard();
    if (dev.bar0 == 0) return;

    virtio_mmio = (volatile uint32_t*)dev.bar0;

    uint32_t magic = virtio_read(0x000);
    uint32_t device_id = virtio_read(0x008);
    if (magic != 0x74726976 || device_id != 0x1050) {
        kprintf("VirtIO device mismatch: magic=0x%x device=0x%x\n", magic, device_id);
        return;
    }

    kprintf("VirtIO Keyboard ready at 0x%x\n", dev.bar0);
}