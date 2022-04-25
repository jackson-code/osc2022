#include "irq.h"

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }


void default_exception_handler()
{
/*
	unsigned long spsr_el1, elr_el1, esr_el1;
	// system register write to the c variable
	asm volatile("mrs	%0, spsr_el1	\n":"=r"(spsr_el1):);
	asm volatile("mrs	%0, elr_el1		\n":"=r"(elr_el1):);
	asm volatile("mrs	%0, esr_el1		\n":"=r"(esr_el1):);
	
	uart_puts("---------------------------\n");
	uart_puts("spsr_el1 = ");
	uart_hex(spsr_el1);
	uart_puts("elr_el1  = ");
	uart_hex(elr_el1);
	uart_puts("esr_el1  = ");
	uart_hex(esr_el1);	
	uart_puts("---------------------------\n");
	*/
	
	uart_puts("default_exception_handler");
	uart_puts("\n");
}

// don't use uart func in this func, avoiding infinite uart interrupt
void handle_el0_irq()
{
	//disable_interrupt();
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if(uart){
        uart_handler();
    }
    else if(core_timer){
        el0_timer_irq();
    }
	//enable_interrupt();
}

void handle_el1_irq()
{
	//disable_interrupt();
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if(uart){
        uart_handler();
    }
    else if(core_timer){// CNTPNIRQ
        el1_timer_irq();
    }
	//enable_interrupt();
}
