#include <kernel/kprint.h>
#include <kernel/uart.h>
#include <kernel/tty.h>
#include <stdarg.h>

void kputc(char c)
{
    //volatile unsigned char *uart = (unsigned char*)UART0;
    //*uart = c;

    if (current_tty) tty_putc(current_tty, c);
}

void kputs(const char *s)
{
    while (*s) {
        kputc(*s++);
    }
}

int kgetc(uint32_t* status)
{
//    if (current_tty) {
//        return (int)uart_getc((struct uart_hardware_interface*) current_tty->tty, status);
//    }
    return -1;
}

static void itoa(int value, char* buffer, int base)
{
    char temp[32];
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    while (value != 0) {
        int rem = value % base;
        temp[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value /= base;
    }

    if (is_negative)
        temp[i++] = '-';

    int j = 0;
    while (i > 0)
        buffer[j++] = temp[--i];
    buffer[j] = '\0';
}

static void itoa_ll(long long value, char* buffer, int base)
{
    char temp[64];
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    while (value != 0) {
        int rem = value % base;
        temp[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value /= base;
    }

    if (is_negative)
        temp[i++] = '-';

    int j = 0;
    while (i > 0)
        buffer[j++] = temp[--i];

    buffer[j] = '\0';
}

void kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0'; i++) {

        if (fmt[i] != '%') {
            kputc(fmt[i]);
            continue;
        }

        i++;

        int long_count = 0;

        while (fmt[i] == 'l') {
            long_count++;
            i++;
        }

        char buffer[64];

        switch (fmt[i]) {

        case 'd':
        case 'i':
            if (long_count == 2) {
                itoa_ll(va_arg(args, long long), buffer, 10);
            } else if (long_count == 1) {
                itoa_ll(va_arg(args, long), buffer, 10);
            } else {
                itoa(va_arg(args, int), buffer, 10);
            }
            kputs(buffer);
            break;

        case 'u':
            if (long_count == 2) {
                itoa_ll(va_arg(args, unsigned long long), buffer, 10);
            } else if (long_count == 1) {
                itoa_ll(va_arg(args, unsigned long), buffer, 10);
            } else {
                itoa(va_arg(args, unsigned int), buffer, 10);
            }
            kputs(buffer);
            break;

        case 'x':
            if (long_count == 2) {
                itoa_ll(va_arg(args, unsigned long long), buffer, 16);
            } else if (long_count == 1) {
                itoa_ll(va_arg(args, unsigned long), buffer, 16);
            } else {
                itoa(va_arg(args, unsigned int), buffer, 16);
            }
            kputs(buffer);
            break;

        case 'c':
            kputc((char)va_arg(args, int));
            break;

        case 's':
            kputs(va_arg(args, char*));
            break;

        default:
            kputc('%');
            kputc(fmt[i]);
            break;
        }
    }

    va_end(args);
}