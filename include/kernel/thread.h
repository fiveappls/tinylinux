#pragma once
#include <kernel/process.h>
#include <stdint.h>

typedef uint8_t tid_t;

typedef enum { T_UNALLOCATED, T_READY, T_RUNNING, T_IDLE, T_DEAD } thread_state_t;

#define MAX_THREADS 256
#define MAX_THREADS_MSK (MAX_THREADS - 1)
#define SCHED_QUANTUM_NS 1000000

struct thread_context 
{
    uintptr_t ra;
    uintptr_t sp;

    uintptr_t s0;
    uintptr_t s1;
    uintptr_t s2;
    uintptr_t s3;
    uintptr_t s4;
    uintptr_t s5;
    uintptr_t s6;
    uintptr_t s7;
    uintptr_t s8;
    uintptr_t s9;
    uintptr_t s10;
    uintptr_t s11;

    uintptr_t tp;
};

struct thread_control_block
{
    struct thread_context ctx;
    uintptr_t entry_point;
    thread_state_t state;

    struct process* owner;
    tid_t tid;

    struct thread_control_block* next;
};

void thread_init(void);
struct thread_control_block* thread_create(struct process* owner, uintptr_t entry_point, uintptr_t stack, _Bool);
_Bool thread_delete(struct thread_control_block* thread);
void thread_yield(void);
void thread_scheduler_init(void);
void thread_schedule(void);