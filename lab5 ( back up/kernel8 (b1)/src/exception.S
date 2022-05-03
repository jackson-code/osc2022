// ----- important system register -----
// SP_EL0 (Stack Pointer):
//      Holds the stack pointer associated with EL0. At higher Exception levels, this is used as the current stack pointer when the value of SPSel.SP is 0.
// ELR_EL1 (Exception Link Register):   
//      When taking an exception to EL1, holds the address to return to. 
// SPSR_EL1 (Saved Program Status Register):
//      Holds the saved process state when an exception is taken to EL1.
// -------------------------------------

// save general registers to stack
.macro save_all
    // trap frame: used for pass args and return value between user and kernel
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]

//    mrs x21, sp_el0
 //   stp x30, x21, [sp, #16 * 15]

 //   mrs x21, elr_el1
 //   mrs x22, spsr_el1
 //   stp x21, x22, [sp, #16 * 16]

    mrs x9, sp_el0
    stp x30, x9, [sp, #16 * 15]

    mrs x10, elr_el1
    mrs x11, spsr_el1
    stp x10, x11, [sp, #16 * 16]
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    //ldr x30, [sp, 16 * 15]

    ldp x30, x21, [sp, 16 * 15]
    msr sp_el0, x21

    ldp x21, x22, [sp, 16 * 16]
    msr spsr_el1, x22
    msr elr_el1, x21

    add sp, sp, 34 * 8
.endm


.macro	exception_handler_entry handler
	.align 7 						// entry size is 0x80, .align will pad 0
	b	\handler 					// branch to a handler function.
.endm

.align 11 							// vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:				// exception handler defined in irq.c 
	// Current EL with SP0
    // exception from the current EL while using SP_EL0
	exception_handler_entry     curr_el_sp0_sync 	
	exception_handler_entry     curr_el_sp0_irq 	
	exception_handler_entry     curr_el_sp0_fiq 	
	exception_handler_entry     curr_el_sp0_serror
	
	// Current EL with SPx
    // exception from the current EL while using SP_ELx
	exception_handler_entry     curr_el_spx_sync 	
	exception_handler_entry     curr_el_spx_irq 	        // have implement
	exception_handler_entry     curr_el_spx_fiq 	
	exception_handler_entry     curr_el_spx_serror 
	
	// Lower EL using AArch64
    // exception from the lower EL at least one lower EL is AArch64
	exception_handler_entry     lower_el_aarch64_sync 	    // have implement (system call)
	exception_handler_entry     lower_el_aarch64_irq 	    // have implement
	exception_handler_entry     lower_el_aarch64_fiq 	
	exception_handler_entry     lower_el_aarch64_serror 
	
	// Lower EL using AArch32
    // exception from the lower EL and all lower EL is AArch32
	exception_handler_entry     lower_el_aarch32_sync 	
	exception_handler_entry     lower_el_aarch32_irq 	
	exception_handler_entry     lower_el_aarch32_fiq 	
	exception_handler_entry     lower_el_aarch32_serror  			
    
lower_el_aarch64_irq:
   	save_all								// save registers for user.S 
	bl		lower_el_aarch64_irq_
	load_all								// load registers for user.S 
    eret									// 

curr_el_spx_irq:
    save_all
    bl 		curr_el_spx_irq_
    load_all
    eret

lower_el_aarch64_sync:
    save_all
    mrs x0, esr_el1
    mrs x1, elr_el1
    mov x2, sp          
    bl 		lower_el_aarch64_sync_
    load_all
    eret


//*********************************************************//
//              imcomplete exception                       //
//*********************************************************//
curr_el_sp0_sync:
    save_all								// save registers for caller
	bl		curr_el_sp0_sync_
	load_all								// load registers for caller 
    eret									// return to caller

curr_el_sp0_irq:
    save_all								
	bl		curr_el_sp0_irq_
	load_all								
    eret

curr_el_sp0_fiq:
    save_all								
	bl		curr_el_sp0_fiq_
	load_all								
    eret	

curr_el_sp0_serror:
    save_all								
	bl		curr_el_sp0_serror_
	load_all								
    eret	

curr_el_spx_sync:
    save_all								
	bl		curr_el_spx_sync_
	load_all								
    eret	

 curr_el_spx_fiq:
    save_all								
	bl		curr_el_spx_fiq_
	load_all								
    eret	

curr_el_spx_serror:
    save_all								
	bl		curr_el_spx_serror_
	load_all								
    eret	

lower_el_aarch64_fiq:
    save_all								
	bl		lower_el_aarch64_fiq_
	load_all								
    eret	

lower_el_aarch64_serror:
    save_all								
	bl		lower_el_aarch64_serror_
	load_all								
    eret	

lower_el_aarch32_sync:
    save_all								
	bl		lower_el_aarch32_sync_
	load_all								
    eret	

lower_el_aarch32_irq:
    save_all								
	bl		lower_el_aarch32_irq_
	load_all								
    eret	

lower_el_aarch32_fiq:
    save_all								
	bl		lower_el_aarch32_fiq_
	load_all								
    eret	

lower_el_aarch32_serror:
    save_all								
	bl		lower_el_aarch32_serror_
	load_all								
    eret			
//*********************************************************//
//*********************************************************//



.global set_exception_vector_table
set_exception_vector_table:
	adr 	x0, exception_vector_table
	msr 	vbar_el1, x0
	ret										// return to caller in start.S
