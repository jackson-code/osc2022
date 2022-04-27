#include "exception.h"

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


void sys_get_task_id(struct trapframe* trapframe) {
    unsigned long task_id = thread_get_current()->id;
    trapframe->x[0] = task_id;
}


void sys_uart_read(struct trapframe* trapframe) {
    char* buf = (char*) trapframe->x[0];
    unsigned long size = trapframe->x[1];
    
    //uart_gets((char*)x0,(int)x1,1);
    uart_get_string(buf, size);
/*
    irq_enable();
    for (unsigned long i = 0; i < size; i++) {
        buf[i] = uart0_read();
    }
    buf[size] = '\0';
    irq_disable();
    */
    trapframe->x[0] = size;
}

void sys_uart_write(struct trapframe* trapframe) {
    char* buf = (char*) trapframe->x[0];
    unsigned long size = trapframe->x[1];

    //irq_enable();
    uart_puts(buf);
    /*
    for (unsigned long i = 0; i < size; i++) {
        uart0_write(buf[i]);
    }*/
    //irq_disable();
    trapframe->x[0] = size;
}


/*
	esr_el1 (Exception Syndrome Register):	
		Holds syndrome information for an exception taken to EL1
	elr_el1 (Exception Link Register):
		When taking an exception to EL1, holds the address to return to.
*/
void lower_el_aarch64_sync_(unsigned long esr_el1, unsigned long elr_el1, struct trapframe* trapframe)
{
	//disable_interrupt();
	//uart_puts("lower_el_aarch64_sync_ \n");

	unsigned long ex =  (esr_el1 >> 26) & 0x3f;	// Exception Class. Indicates the reason for the exception that this register holds information about.
	if (ex == 0b010101) // SVC instruction execution in AArch64 state
	{
		unsigned long sys_call_num = trapframe->x[8];
		//sys_call_router(sys_call_num, trapframe);
		switch (sys_call_num) {
        case SYS_GET_PID:
            sys_get_task_id(trapframe);
            break;

        case SYS_UART_READ:
            sys_uart_read(trapframe);
            break;

        case SYS_UART_WRITE:
            sys_uart_write(trapframe);
            break;
		}
	} else {
		/* code */
	}
	

	//enable_interrupt();
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