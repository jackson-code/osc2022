.section ".text"
.global _start
_start:	

	// clear bss
	ldr 	x1, =__bss_start
	ldr 	x2, =__bss_end

	cmp 	x1, x2
	blo 	clear_bss

clear_bss:
	str 	wzr, [x1], #4		// post-indexed
	cmp 	x1, x2
	blo 	clear_bss	
	bl 		main	
