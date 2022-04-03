#include "el1.h"

void el_user_start()
{
	char *file_addr = cpio_get_addr("user.img");
	uart_puts("user address : ");
	uart_hex((unsigned long)file_addr);


/*
	int app_size = find_app_size("user.img");
	uart_puts("user prog size : ");
	uart_put_int(app_size);

	char *target = NEW_ADDR;
	while(app_size--){
		*target=*file_addr;
		target++;
		file_addr++;
	}
	*/
	uart_puts("loading user prog...\n");

	if (!str_cmp("no file", file_addr)) {
		uart_puts("ERROR: no file");
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
		asm volatile("eret					\n");
	}
}

