ENTRY(_start)

SECTIONS
{
    . = 1M;

    _kernel_start = .;

    .multiboot.data : ALIGN(4K)
    {
        *(.multiboot.data)
    }

    .multiboot.text : ALIGN(4K)
    {
        *(.multiboot.text)
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
        *(.data)
    }

    .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;

    .vga_buffer 0xC03FF000 (NOLOAD) : {
        vga_buffer = .;
    }
}
