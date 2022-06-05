.section ".text"
.global _start
_start:	


//jump to main
	bl main//function call


//bss init
	ldr x1, =__bss_start//64-bit load
	ldr w2, =__bss_size//32-bit load
3:
	cbz w2, 4f
	str xzr, [x1], #8//64-bit store
	sub w2, w2, #1
	b 3b
4: