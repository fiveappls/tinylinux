#include <kernel/kprint.h>
#include <kernel/pci.h>

const int debug = 1;

volatile uint32_t* pci_mmio = (volatile uint32_t*)PCI_CONFIG_BASE;

static inline uint32_t pci_calc_addr(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return ( (uint32_t)bus  << 20 ) |
           ( (uint32_t)slot << 15 ) |
           ( (uint32_t)func << 12 ) |
           ( offset & 0xFFF );
}

uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = pci_calc_addr(bus, slot, func, offset);
    if (debug) kprintf("Addr %d : ", addr);
    if (debug) kprintf("pci_mmio[ addr / 4 ] %d : ", pci_mmio[ addr / 4 ]);
    return pci_mmio[ addr / 4 ];
}

void pci_config_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t addr = pci_calc_addr(bus, slot, func, offset);
    pci_mmio[ addr / 4 ] = value;
}

#define VIRTIO_PCI_VENDOR_ID 0x1AF4
#define VIRTIO_DEVICE_KEYBOARD 0x1050

pci_device pci_scan_virtio_keyboard(void) {
    pci_device dev = {0,0,0,0};

    if (debug) kprintf("Scanning PCI bus 0 for devices...\n");

    for (uint8_t slot = 0; slot < 32; slot++) {
        if (debug) kprintf("Bus 0 : Slot %d : ", slot);
        uint32_t val = pci_config_read(0, slot, 0, 0);
        if (debug) kprintf("Val %d : ", val);
        uint16_t vendor = (uint16_t)(val & 0xFFFF);
        if (debug) kprintf("Vendor %d : ", vendor);
        uint16_t device = (uint16_t)((val >> 16) & 0xFFFF);
        if (debug) kprintf("Device %d\n", device);

        //if (debug) kprintf("Bus 0 : Slot %d Vendor: %d Device: %d\n", slot, vendor, device);

        if (vendor == 0xFFFF || vendor == 0x0000) continue;

        if (vendor != VIRTIO_PCI_VENDOR_ID) continue;
        if (device != VIRTIO_DEVICE_KEYBOARD) continue;

        uint32_t bar0 = pci_config_read(0, slot, 0, 0x10);
        dev.bus = 0;
        dev.slot = slot;
        dev.function = 0;
        dev.bar0 = bar0 & ~0xF;
        kprintf("Found VirtIO Keyboard at BAR0 = 0x%x\n", dev.bar0);
        return dev;
    }

    kprintf("VirtIO Keyboard not found!\n");
    return dev;
}