#pragma once
#include <kernel/thread.h>
#include <stdint.h>

#define MAX_PROCESSES 256
#define MAX_PROCESSES_MSK (MAX_PROCESSES - 1)

typedef uint8_t pid_t;

typedef enum { P_RUNNING, P_READY, P_BLOCKED, P_DEAD, P_UNALLOCATED } process_state_t;

struct process 
{
    struct thread_control_block* thread;
    process_state_t state;

    pid_t pid;
    
    uintptr_t program_base;
    uintptr_t program_size;
};

void process_init(void);
struct process* process_create(uint32_t entry_point, uintptr_t stack_pointer);
_Bool process_delete(struct process* process);
void process_yield(struct process* process);
void process_update(void);