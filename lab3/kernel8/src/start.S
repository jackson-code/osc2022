.section ".text"

// let linker.ld start from _start
.global _start

_start:
    ldr		x1, =0x100000
    str		x0, [x1]            // store dtb loading address to 0x100000

	bl 		from_el2_to_el1	
	bl		set_exception_vector_table
	
	// set stack pointer
	ldr		x0, = __end
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
	bl 		main	
	

from_el2_to_el1:
    mov 	x0, (1 << 31) 		// EL1 uses aarch64
    msr 	hcr_el2, x0			// write Xn to the system register(Hypervisor Configuration Register).
    mov 	x0, 0x3c5 			// EL1h (SPSel = 1) with interrupt disabled	
    msr 	spsr_el2, x0
    msr 	elr_el2, lr
    eret 						// return to EL1
    



    
	

