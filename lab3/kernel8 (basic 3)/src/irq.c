#include "irq.h"

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }


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

// don't use uart func in this func, avoiding infinite uart interrupt
void handle_el0_irq()
{
	disable_interrupt();
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if(uart){
        uart_handler();
    }
    else if(core_timer){
        el0_timer_irq();
    }
	enable_interrupt();
}

void handle_el1_irq()
{
	disable_interrupt();
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if(uart){
        uart_handler();
    }
    else if(core_timer){// CNTPNIRQ
        el1_timer_irq();
    }
	enable_interrupt();
}
