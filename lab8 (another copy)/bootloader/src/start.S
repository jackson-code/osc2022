.section ".text"

// let linker.ld start from _start
.global _start

_start:
    ldr		x1, =0x100000
    str		x0, [x1]            // store dtb loading address to 0x100000

	// set stack pointer
	ldr		x0, = __stack_top
	mov		sp, x0
	
	// clear bss
	ldr 	x1, =__bss_start
	ldr 	x2, =__bss_end
	
	cmp 	x1, x2
	blo 	clear_bss

clear_bss:
	str 	wzr, [x1], #4		// post-indexed
	cmp 	x1, x2
	blo 	clear_bss
	
	// branch to main function.
	bl 		main	
	
	
//1:
//	wfe
//	b		1b
	

