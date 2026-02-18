#pragma once
#include <stdint.h>

#define CLINT_MTIME     ((volatile uint64_t*)0x200BFF8)
#define CLINT_MTIMECMP  ((volatile uint64_t*)0x2004000)

uint64_t timer_get_ticks(void);
uint64_t timer_get_ns(void);
void timer_init();