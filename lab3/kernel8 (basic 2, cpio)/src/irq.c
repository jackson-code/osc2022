#include "irq.h"

void default_exception_handler()
{
	unsigned long spsr_el1_, elr_el1_, esr_el1_;
	// system register write to the c variable
	asm volatile("mrs	%0, spsr_el1	\n":"=r"(spsr_el1_):);
	asm volatile("mrs	%0, elr_el1		\n":"=r"(elr_el1_):);
	asm volatile("mrs	%0, esr_el1		\n":"=r"(esr_el1_):);
	
	// debug
	//unsigned long x0;
	//asm volatile("mov	%0, x0		\n":"=r"(x0):);
	
	uart_puts("-------------EL1--------------");
	uart_puts("spsr_el1 = ");
	uart_hex(spsr_el1_);
	uart_puts("elr_el1  = ");
	uart_hex(elr_el1_);
	uart_puts("esr_el1  = ");
	uart_hex(esr_el1_);	
	
	//uart_puts("x0 = ");
	//uart_hex(x0);	
		
	uart_puts("-------------EL1--------------");
}

void handle_el0_irq()
{
	//uart_puts("handle_el0_irq");
	
	if(0)
	{
	
	}
	else
	{
		el0_timer_irq();
	}
}

void handle_el1_irq()
{
	uart_puts("handle_el1_irq");

	if(0)
	{
	
	}
	else
	{
		el1_timer_irq();
	}
}
