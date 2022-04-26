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
	
	uart_puts("irq ");
	//uart_puts("\n");
}

// don't use uart func in this func, avoiding infinite uart interrupt
void lower_el_aarch64_irq_()
{
	disable_interrupt();
	//uart_puts("lower_el_aarch64_irq_ \n");
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

void curr_el_spx_irq_()
{
	disable_interrupt();
	//uart_puts("curr_el_spx_irq_ \n");
    unsigned int uart = (*IRQ_PENDING_1 & AUX_IRQ);
    unsigned int core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);
    if(uart){
        uart_handler();
    }
    else if(core_timer){// CNTPNIRQ
        timer_el1_irq();
    }
	enable_interrupt();
}

void lower_el_aarch64_sync_()
{
	disable_interrupt();
	//uart_puts("lower_el_aarch64_sync_ \n");
	enable_interrupt();
}


//*********************************************************//
//              imcomplete exception                       //
//*********************************************************//
void curr_el_sp0_sync_()
{
	uart_puts("curr_el_sp0_sync_ ");
}

void curr_el_sp0_irq_()
{
	uart_puts("curr_el_sp0_irq_ ");
}

void curr_el_sp0_fiq_()
{
	uart_puts("curr_el_sp0_fiq_ ");
}
void curr_el_sp0_serror_()
{
	uart_puts("curr_el_sp0_serror_ ");
}
void curr_el_spx_sync_()
{
	uart_puts("curr_el_spx_sync_ ");
}
void curr_el_spx_fiq_()
{
	uart_puts("curr_el_spx_fiq_ ");
}
void curr_el_spx_serror_()
{
	uart_puts("curr_el_spx_serror_ ");
}
void lower_el_aarch64_fiq_()
{
	uart_puts("lower_el_aarch64_fiq_ ");
}
void lower_el_aarch64_serror_()
{
	uart_puts("lower_el_aarch64_serror_ ");
}
void lower_el_aarch32_sync_()
{
	uart_puts("lower_el_aarch32_sync_ ");
}
void lower_el_aarch32_irq_()
{
	uart_puts("lower_el_aarch32_irq_ ");
}
void lower_el_aarch32_fiq_()
{
	uart_puts("lower_el_aarch32_fiq_ ");
}
void lower_el_aarch32_serror_()
{
	uart_puts("lower_el_aarch32_serror_ ");
}
//*********************************************************//
//*********************************************************//
//*********************************************************//