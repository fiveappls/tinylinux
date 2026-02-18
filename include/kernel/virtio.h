#pragma once
#include <stdint.h>

#define VIRTIO_BASE 0x10001000

void virtio_init(void);

void virtio_write(uint32_t offset, uint32_t value);
void virtio_write(uint32_t offset, uint32_t value);