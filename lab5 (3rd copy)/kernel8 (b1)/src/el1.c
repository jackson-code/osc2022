#include "el1.h"

void el1_swich_to_el0(char * img_name)
{
	uart_puts("el1_swich_to_el0() begin\n");

	char *file_addr = cpio_get_addr(img_name);		// lab3
	unsigned long app_size = find_app_size(img_name);
	//char *file_addr = cpio_get_addr("app2.img");		// lab5
	//unsigned long app_size = find_app_size("app2.img");
	uart_puts("user address : ");
	uart_put_hex((unsigned long)file_addr);
	uart_puts("user prog size : ");
	uart_put_int(app_size);

/*
	unsigned long page_count = ((app_size-1) / 4096) + 1;
	char *new_addr = kmalloc(page_count * 4096);
	uart_puts("\nmoving user prog...\n");
	while(app_size--){
		*new_addr = *file_addr;
		new_addr++;
		file_addr++;
	}
	*/
	
	uart_puts("loading user prog...\n");

	if (!str_cmp("no file", file_addr)) {
		uart_puts("ERROR in el1.c, el1_swich_to_el0(): no file");
	}
	else {
		uart_puts("switch EL1 to EL0...");
		
		assert_receive_interrupt();
		assert_transmit_interrupt();
		
		//asm volatile("mov	x0, 0x3c0		\n");		// using sp_el0, interrupt disable
		asm volatile("mov	x0, 0x340		\n");		// using sp_el0, interrupt enable
		asm volatile("msr	spsr_el1, x0	\n");
		
		// set stack pointer
		asm volatile("ldr	x1, =0x160000	\n");
		asm volatile("msr	sp_el0, x1		\n");
		
		// set return address, return to the user.S
		asm volatile("msr	elr_el1, %0		\n"::"r"(file_addr));
		//asm volatile("msr	elr_el1, %0		\n"::"r"(new_addr));
		asm volatile("eret					\n");
	}
}

