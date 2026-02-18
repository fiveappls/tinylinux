ENTRY(_start)

SECTIONS
{
    . = 0x80000000;

    .text : { *(.text*) }

    .rodata : { *(.rodata*) }

    .data : { *(.data*) }

    .bss : {
        *(.bss*)
        *(COMMON)
    }

    . = ALIGN(16);
    _end = .;
    _ram_end = 0x88000000;
    _ram_start = 0x80000000;
    _stack_top = 0x80040000;
}