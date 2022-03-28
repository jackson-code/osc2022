#include "irq.h"

void default_exception_handler()
{
	unsigned long spsr_el1_, elr_el1_, esr_el1_;
	// system register write to the c variable
	asm volatile("mrs	%0, spsr_el1	\n":"=r"(spsr_el1_):);
	asm volatile("mrs	%0, elr_el1		\n":"=r"(elr_el1_):);
	asm volatile("mrs	%0, esr_el1		\n":"=r"(esr_el1_):);
	
	uart_puts("-------------EL1--------------");
	uart_puts("= SPSR : ");
	uart_hex(spsr_el1_);
	uart_puts("= ELR  : ");
	uart_hex(elr_el1_);
	uart_puts("= ESR  : ");
	uart_hex(esr_el1_);		
	uart_puts("-------------EL1--------------");
}

