#include <kernel/uart.h>

void uart_putc(struct uart_hardware_interface* uart, char c)
{
    //while (!(*uart->status & 0x01));
    *uart->data = c;
}