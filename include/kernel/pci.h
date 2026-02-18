#pragma once
#include <stdint.h>

#define PCI_CONFIG_BASE 0x10000000UL

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    uint32_t bar0;
} pci_device;

void pci_config_write(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value);
static inline uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);

pci_device pci_scan_virtio_keyboard(void);