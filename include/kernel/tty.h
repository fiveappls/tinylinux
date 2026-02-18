#pragma once
#include <stdint.h>
#include <kernel/device.h>

struct tty_hardware_interface {
    char input_character;
    uint8_t input_ready;
    uint8_t clear;
    uint8_t write;
    char character;
    uint8_t cursor_location;
};

struct tty_device {
    struct device base;
    volatile struct tty_hardware_interface* tty;
    struct device_request* current_req;
    uint32_t current_bytes_copied;
};

extern struct tty_device* current_tty;

void tty_init();
struct device* tty_create(int8_t* minor, const void* args);
void tty_destroy(uint8_t minor);
void tty_update(struct device* dev);
void tty_global_update();
void tty_putc(struct tty_device* tty, char c);
static inline uint8_t tty_write();
static inline uint8_t tty_read();
struct device* tty_lookup(uint8_t minor);