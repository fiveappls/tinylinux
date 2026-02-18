#include <kernel/kpanic.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <stdint.h>
#include <stddef.h>

struct thread_control_block* current_thread = NULL;

struct thread_control_block thread_table[MAX_THREADS];
static struct thread_control_block* ready_queue[MAX_THREADS];
static uint8_t ready_queue_head;
static uint8_t ready_queue_tail;
static uint8_t ready_queue_count;

static uint8_t idle_stack[256] __attribute__((aligned(16)));
static struct thread_control_block* idle_thread = NULL;

static uint64_t next_preempt_ns = 0;

static void switch_to_task(struct thread_control_block* old, struct thread_control_block* new)
{
    if (!new) return;
    if (!old) 
    {
        asm volatile(
            "lw ra, 0(%0)\n\t"
            "lw sp, 8(%0)\n\t"
            "ret\n\t"
            :
            : "r"(&new->ctx)
        );
    }

    asm volatile(
        "sw ra, 0(%0)\n\t"
        "sw sp, 8(%0)\n\t"

        "sw s0, 16(%0)\n\t"
        "sw s1, 24(%0)\n\t"
        "sw s2, 32(%0)\n\t"
        "sw s3, 40(%0)\n\t"
        "sw s4, 48(%0)\n\t"
        "sw s5, 56(%0)\n\t"
        "sw s6, 64(%0)\n\t"
        "sw s7, 72(%0)\n\t"
        "sw s8, 80(%0)\n\t"
        "sw s9, 88(%0)\n\t"
        "sw s10, 96(%0)\n\t"
        "sw s11, 104(%0)\n\t"

        "sw tp, 112(%0)\n\t"

        "lw ra, 0(%1)\n\t"
        "lw sp, 8(%1)\n\t"

        "lw s0, 16(%1)\n\t"
        "lw s1, 24(%1)\n\t"
        "lw s2, 32(%1)\n\t"
        "lw s3, 40(%1)\n\t"
        "lw s4, 48(%1)\n\t"
        "lw s5, 56(%1)\n\t"
        "lw s6, 64(%1)\n\t"
        "lw s7, 72(%1)\n\t"
        "lw s8, 80(%1)\n\t"
        "lw s9, 88(%1)\n\t"
        "lw s10, 96(%1)\n\t"
        "lw s11, 104(%1)\n\t"

        "lw tp, 112(%1)\n\t"

        "ret\n\t"
        :
        : "r"(&old->ctx), "r"(&new->ctx)
        : "memory"
    );
}

static void idle_thread_entry(void)
{
    while (1) {
        asm volatile("wfi");
    }
}

_Bool thread_enqueue(struct thread_control_block* thread) 
{
    if (ready_queue_count == MAX_THREADS) return 1;

    ready_queue[ready_queue_tail] = thread;
    ready_queue_tail = (ready_queue_tail + 1) & MAX_THREADS_MSK;
    ready_queue_count++;

    return 0;
}

struct thread_control_block* thread_dequeue(void)
{
    if (ready_queue_count == 0) return NULL;
    
    struct thread_control_block* thread = ready_queue[ready_queue_head];
    ready_queue_head = (ready_queue_head + 1) & MAX_THREADS_MSK;
    ready_queue_count--;
    
    return thread;
}

void thread_init(void)
{
    ready_queue_head = 0;
    ready_queue_tail = 0;
    ready_queue_count = 0;

    for (int i = 0; i < MAX_THREADS; i++) {
        thread_table[i].entry_point = 0;
        thread_table[i].next = NULL;
        thread_table[i].state = T_UNALLOCATED;
        thread_table[i].tid = i;
    }

    idle_thread = thread_create(NULL, (uintptr_t)idle_thread_entry, (uintptr_t)(idle_stack + sizeof(idle_stack)), 1);
    idle_thread->state = T_IDLE;
    if (!idle_thread) kpanic("failed to create idle thread\n");
}

struct thread_control_block* thread_create(struct process* owner, uintptr_t entry, uintptr_t stack_top, _Bool idle) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i].state != T_UNALLOCATED) continue;
        struct thread_control_block* new_thread = &thread_table[i];

        new_thread->entry_point = entry;
        new_thread->ctx.sp = stack_top;
        new_thread->owner = owner;
        new_thread->state = T_READY;

        new_thread->ctx.sp = stack_top;
        new_thread->ctx.ra = entry;
        new_thread->ctx.tp = (uintptr_t)new_thread;

        if (!idle) thread_enqueue(new_thread);
        return new_thread;
    }

    return NULL;
}

_Bool thread_delete(struct thread_control_block* thread) {
    if (thread->state == T_DEAD) {
        thread->state = T_UNALLOCATED;
        return 0;
    }
    return 1;
}

void thread_yield(void) {
    if (!current_thread) return;

    current_thread->state = T_READY;
    thread_enqueue(current_thread);

    struct thread_control_block* next = thread_dequeue();
    if (!next) return;

    struct thread_control_block* old = current_thread;
    current_thread = next;
    current_thread->state = T_RUNNING;

    switch_to_task(old, next);
}

void thread_scheduler_init(void) {
    next_preempt_ns = timer_get_ns() + SCHED_QUANTUM_NS;
}

void thread_scheduler_tick(void) {
    uint64_t now = timer_get_ns();

    if (now >= next_preempt_ns) {
        next_preempt_ns = now + SCHED_QUANTUM_NS;
        thread_schedule();
    }
}

void thread_schedule(void) {
    struct thread_control_block* next_thread = thread_dequeue();
    if (!next_thread) {
        next_thread = idle_thread;
    }

    if (current_thread) {
        if (current_thread->state == T_RUNNING) current_thread->state = T_READY;
        thread_enqueue(current_thread);
    }

    struct thread_control_block* old_thread = current_thread;
    current_thread = next_thread;
    next_thread->state = T_RUNNING;

    switch_to_task(old_thread, next_thread);
}