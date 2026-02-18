#pragma once
#include <stdint.h>

struct uart_hardware_interface {
    volatile uint8_t* data;
    volatile uint8_t* status;
};

void uart_putc(struct uart_hardware_interface* uart, char c);
char uart_getc(struct uart_hardware_interface* uart, uint32_t* status);