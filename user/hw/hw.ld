OUTPUT_FORMAT("elf32-i386", "elf32-i386",
	      "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(start)

SECTIONS
{
    . = 0x2800000;
    .text :
    {
        *(.text*);
    }
    .data :
    {
        *(.data*);
        *(.bss*);
        *(.rodata*);
    }
    /DISCARD/ :
    {
        *(*);
    }
}
