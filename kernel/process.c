#include <kernel/process.h>
#include <kernel/device.h>
#include <kernel/kpanic.h>
#include <kernel/thread.h>
#include <stddef.h>

struct process process_table[MAX_PROCESSES];

void process_init(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = P_UNALLOCATED;
        process_table[i].pid = i;
    }
}

struct process* process_create(uint32_t entry_point, uintptr_t stack_pointer) 
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != P_UNALLOCATED) continue;
        struct process* new_process = &process_table[i];
        struct thread_control_block* new_thread = thread_create(new_process, entry_point, stack_pointer, 0);
        if (!new_thread) return NULL;
        new_process->thread = new_thread;
        new_process->state = P_READY;

        new_process->program_base = 0;
        new_process->program_size = 0;
        
        return new_process;
    }

    return NULL;
}

_Bool process_delete(struct process* process) {
    if (process->state == P_DEAD) {
        process->state = P_UNALLOCATED;
        struct thread_control_block* process_thread = process->thread;
        if (process_thread) {
            process_thread->state = T_DEAD;
            thread_delete(process_thread);
        }
        return 0;
    }
    return 1;
}

void process_yield(struct process* process)
{
    thread_yield();
}