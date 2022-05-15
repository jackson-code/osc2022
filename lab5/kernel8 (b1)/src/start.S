.section ".text"

// let linker.ld start from _start
.global _start

_start:
    ldr		x1, =0x100000
    str		x0, [x1]            // store dtb loading address to 0x100000

	bl 		from_el2_to_el1	
	bl		set_exception_vector_table
	
//bss init
	ldr x1, =__bss_start//64-bit load
	ldr w2, =__bss_size//32-bit load
3:
	cbz w2, 4f
	str xzr, [x1], #8//64-bit store
	sub w2, w2, #1
	b 3b

4:
//stack init
	ldr x1, =__stack_top
	mov sp, x1

//jump to main
	bl main//function call
	

from_el2_to_el1:
    mov 	x0, (1 << 31) 		// EL1 uses aarch64
    msr 	hcr_el2, x0			// write Xn to the system register(Hypervisor Configuration Register).
    mov 	x0, 0x3c5 			// 5: EL1h (SPSel = 1, means using sp_el2?), with interrupt disabled	
    msr 	spsr_el2, x0
    msr 	elr_el2, lr
    eret 						// return to EL1
    



    
	

