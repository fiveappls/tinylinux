#pragma once
#include <stddef.h>
#include <stdint.h>

static inline void memset(void* ptr, int value, size_t size)
{
    uint8_t* p = (uint8_t*)ptr;

    while (size--) *p++ = (uint8_t)value;
}