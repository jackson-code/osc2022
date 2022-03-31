SECTIONS
{
    . = 0x80000;
    __start = .;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }
    .heap : {
    	*(.heap)
    	__heap_base = .;
    	. += 0x8000;		/* heap size */
    	__heap_top = .;
    }
    .stack : {
    	*(.stack)
    	. += 0x8000;		/* stack size */
    	__stack_top = .;
    }
    __end = .;
}
