#include <kernel/timer.h>
#include <stdint.h>

uint64_t TIMER_FREQ = 1;

void timer_init()
{
    TIMER_FREQ = 10000000;
}

uint64_t timer_get_ticks(void)
{
    return *CLINT_MTIME;
}

uint64_t timer_get_ns(void)
{
    return (timer_get_ticks() * 1000000000ULL) / TIMER_FREQ;
}

uint64_t timer_get_ms(void)
{
    return timer_get_ticks() / (TIMER_FREQ / 1000);
}