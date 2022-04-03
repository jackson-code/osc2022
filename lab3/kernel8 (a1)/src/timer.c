#include "timer.h"


void timer_time_after_booting()
{
	core_timer_enable();			// defined in timer.S
	set_expired_time(2);			// 2 seconds
}

void set_timeout()
{
	add_timer();
}

void add_timer()
{
	unsigned int duration = 2;		// in seconds

	// queue be added first
	if(1)
	{
		//el1_interrupt_enable();
		core_timer_enable();			// defined in timer.S
		set_expired_time(duration);
	}
	else
	{
	
	}
}

void set_expired_time(unsigned int duration)
{
	unsigned long cntfrq_el0;
	asm volatile("mrs	%0, cntfrq_el0" : "=r"(cntfrq_el0));				// get frequency of the system counter
	asm volatile("msr	cntp_tval_el0 ,%0" :: "r"(cntfrq_el0 * duration));
}

unsigned long get_current_time() {
  unsigned long cntpct_el0, cntfrq_el0;
  asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct_el0));
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
  return cntpct_el0 / cntfrq_el0;
}


/********************************************************/
//                      irq								//
/********************************************************/
void el1_timer_irq(){
  	uart_puts("el1 Current time: ");
  	uart_put_int(get_current_time());
  	uart_puts("");
	set_expired_time(2);
  	/*
  	uart_puts("s, ");
  	uart_puts("Command executed time: ");
  	uart_put_int(timeout_queue_head->register_time);
  	uart_puts("s, ");
  	uart_puts("Duration: ");
  	uart_put_int(timeout_queue_head->duration);
  	uart_puts("s\n");
    timeout_queue_head->callback(timeout_queue_head->args);
  	timeout_event *next = timeout_queue_head->next;
  	kfree(timeout_queue_head);
  	if (next) {
    	next->prev = 0;
    	timeout_queue_head = next;
    	unsigned long next_duration = next->register_time + next->duration - get_current_time();
    	set_expired_time(next_duration);
  	} 
	else {
    	timeout_queue_head = 0;
    	timeout_queue_tail = 0;
    	core_timer_disable();
  	}
  	*/
}

void el0_timer_irq(){	
  	uart_puts("(el0_timer_irq) Current time: ");
  	uart_put_int(get_current_time());
  	set_expired_time(10);				// timer interrupt each 2 seconds
}
