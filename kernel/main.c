#include <kernel/mm/kmalloc.h>
#include <kernel/process.h>
#include <kernel/mm/pmm.h>
#include <kernel/mm/vmm.h>
#include <kernel/kprint.h>
#include <kernel/virtio.h>
#include <kernel/kpanic.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <kernel/uart.h>
#include <kernel/tty.h>
#include <stdint.h>

#define UART0_DATA   ((volatile uint8_t*)0x10000000)
#define UART0_STATUS ((volatile uint8_t*)0x10000005)

#define UART0_ ((volatile uint8_t*)0x10000000)

void kmain(void) 
{
    //volatile unsigned char *uart_ = (unsigned char*)UART0_;
    //*uart_ = '1';
    //while(1);

    tty_init();
    kputs("tinylinux kernel 0.0.0\nbooting...\n");
    //virtio_init();
    kputs("tty initialized\n");
    timer_init();
    kputs("timer initialized\n");
    pmm_init();
    kputs("pmm initialized\n");
    vmm_init();
    kputs("vmm initialized\n");
    kmalloc_init();
    kputs("kmalloc initialized\n");
    thread_init();
    thread_scheduler_init();
    kputs("thread system initialized\n");
    process_init();
    kputs("process system initialized\n");

    while (1) {
        
    }
}