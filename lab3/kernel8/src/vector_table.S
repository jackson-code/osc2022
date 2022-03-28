

.macro	exception_handler_entry handler
	b	\handler 					// branch to a handler function.
	.align 7 						// entry size is 0x80, .align will pad 0
.endm

.align 11 							// vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler
	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 
	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 
	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler 	
	exception_handler_entry exception_handler  			

exception_handler:
	bl		default_exception_handler

.global set_exception_vector_table
set_exception_vector_table:
	adr 	x0, exception_vector_table
	msr 	vbar_el1, x0
