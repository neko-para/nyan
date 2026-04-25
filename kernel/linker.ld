ENTRY(_start)

SECTIONS
{
    . = 1M;

    _kernel_start = .;

    .multiboot.data : ALIGN(4K)
    {
        KEEP(*(.multiboot.data))
    }

    .multiboot.text : ALIGN(4K)
    {
        KEEP(*(.multiboot.text))
    }

    . += 0xC0000000;

    .text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
    {
        *(.text*)
    }

    .rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
    {
        *(.rodata*)
    }

    .data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
    {
        *(.data*)
    }

    .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss*)
    }

    .trampoline ALIGN(4) : AT (ADDR (.trampoline) - 0xC0000000)
    {
        __trampoline_load_start = .;
        *(.trampoline)
        __trampoline_load_end = .;
    }

    _end = .;

    . = 0xC0800000;

    .vga_buffer 0xC07FF000 (NOLOAD) : {
        vga_buffer = .;
    }
}
