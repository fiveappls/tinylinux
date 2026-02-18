#pragma once
#include <stdint.h>

#define UART0 0x10000000L

void kputc(char c);
void kputs(const char *s);
void kprintf(const char *fmt, ...);
int  kgetc(uint32_t* status);