#include <stdint.h>

#define UART0 0x10000000UL

static inline void uart_putc(char c)
{
    volatile uint8_t *uart = (uint8_t*)UART0;
    *uart = c;
}

static void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

void kpanic(const char *message)
{
    uart_puts("\nKERNEL PANIC: ");
    uart_puts(message ? message : "No reason provided.");
    uart_puts("\n");

    asm volatile("csrci mstatus, 8");
    while (1) asm volatile("wfi");
}