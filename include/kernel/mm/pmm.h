#pragma once
#include <stdbool.h>
#include <stddef.h>

//#define RAM_START 0x80000000
#define PAGE_SIZE 4096
#define MAX_PAGES 32768

void pmm_init(void);
void* pmm_alloc_page(void);
bool pmm_free_page(void* page);