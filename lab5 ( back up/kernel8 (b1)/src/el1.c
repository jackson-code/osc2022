#include "el1.h"

void el1_switch_to_el0(char * img_name)
{
	uart_puts("el1_switch_to_el0() begin\n");

	char *file_addr = cpio_get_addr(img_name);		
	unsigned long app_size = find_app_size(img_name);
	uart_puts("user address : ");
	uart_put_hex((unsigned long)file_addr);
	uart_puts("user prog size : ");
	uart_put_int(app_size);
	uart_puts("\n");


	//unsigned long page_count = ((app_size-1) / 4096) + 1;
	//char *new_addr = kmalloc(page_count * 4096);
	/*
	char *new_addr = (char *)0x50000;
	uart_puts("\nmoving user prog...\n");
	while(app_size--){
		*new_addr = *file_addr;
		new_addr++;
		file_addr++;
	}
	
	uart_puts("loading user prog...\n");

	if (!str_cmp("no file", file_addr)) {
		uart_puts("ERROR in el1.c, el1_switch_to_el0(): no file");
	}
	else {
		uart_puts("switch EL1 to EL0 begin\n");
		
		// memory used by user
		//unsigned long user_stack_top = (unsigned long)kmalloc(4096) + 4000;
		unsigned long user_stack_top = 0x60000;

		assert_receive_interrupt();
		assert_transmit_interrupt();
		
		uart_puts("\t set spsr_el1\n");
		//asm volatile("mov	x0, 0x3c0		\n");		// using sp_el0, interrupt disable
		asm volatile("mov	x0, 0x340		\n");		// using sp_el0, interrupt enable
		asm volatile("msr	spsr_el1, x0	\n");
		
		// set stack pointer
		uart_puts("\t set sp_el0\n");
		asm volatile("msr	sp_el0, %0		\n"::"r"(user_stack_top));
		
		// set return address, return to the user.S
		uart_puts("\t set elr_el1\n");
		asm volatile("msr	elr_el1, %0		\n"::"r"(new_addr));
		//asm volatile("msr	elr_el1, %0		\n"::"r"(new_addr));
		asm volatile("eret					\n");

	}

*/

	// have to moving user.img from cpio to memory(buddy system reserve for user.img)
	// otherwise can't get correct buf address of uartwrite
	char *new_addr = (char *)0x30000;
	char *new_addr_start = (char *)0x30000;
	uart_puts("\nmoving user prog...\n");
	while(app_size--){
		*new_addr = *file_addr;
		new_addr++;
		file_addr++;
	}

	uart_puts("loading user prog...\n");

	if (!str_cmp("no file", file_addr)) {
		uart_puts("ERROR in el1.c, el1_switch_to_el0(): no file");
	}
	else {
		uart_puts("switch EL1 to EL0 begin\n");
		
		// memory used by user
		//unsigned long user_stack_top = (unsigned long)kmalloc(4096) + 4000;
		unsigned long user_stack_top = 0x48000;

		assert_receive_interrupt();
		assert_transmit_interrupt();
		
		uart_puts("\t set spsr_el1\n");
		//asm volatile("mov	x0, 0x3c0		\n");		// using sp_el0, interrupt disable
		asm volatile("mov	x0, 0x340		\n");		// using sp_el0, interrupt enable
		asm volatile("msr	spsr_el1, x0	\n");
		
		// set stack pointer
		uart_puts("\t set sp_el0\n");
		asm volatile("msr	sp_el0, %0		\n"::"r"(user_stack_top));
		
		// set return address, return to the user.S
		uart_puts("\t set elr_el1\n");
		//asm volatile("msr	elr_el1, %0		\n"::"r"(file_addr));
		asm volatile("msr	elr_el1, %0		\n"::"r"(new_addr_start));
		asm volatile("eret					\n");

	}
	
}

