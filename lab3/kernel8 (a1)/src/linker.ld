SECTIONS
{
    . = 0x80000;
    __start = .;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    . = ALIGN(0x1000);		/* why need align ? */
    .data : { *(.data) }
    . = ALIGN(0x1000);		/* why need align ? */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }
    . = ALIGN(0x1000);
    .heap : {
    	*(.heap)
    	__heap_base = .;
    	. += 0x8000;		/* heap size */
    	__heap_top = .;
    }
    . = ALIGN(0x1000);
    .stack : {
    	*(.stack)
    	. += 0x8000;		/* stack size */
    	__stack_top = .;
    }
    __end = .;
}
