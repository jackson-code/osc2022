#include "cpio.h"
#include "uart.h"

void load_user_program()
{
	char *file_addr = cpio_get_addr("user.S");
	uart_puts("APP address : ");
	uart_hex((unsigned long)file_addr);

	if	(*file_addr != -1)
	{
		uart_puts("switch EL1 to EL0...");
		asm volatile("mov	x0, 0x3c0		\n");
		asm volatile("msr	spsr_el1, x0	\n");
		// set stack pointer
		asm volatile("ldr	x1, =0x20000	\n");
		asm volatile("msr	sp_el0, x1		\n");
		// set return address, return to the user.S
		asm volatile("msr	elr_el1, %0		\n"::"r"(file_addr));
		asm volatile("eret\n");
	}
}
