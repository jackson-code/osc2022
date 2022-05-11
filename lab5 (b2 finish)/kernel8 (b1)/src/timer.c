#include "timer.h"
#include "process.h"
#include "scheduler.h"

//extern circular_queue rq_proc;
//circular_queue dq_proc;
//circular_queue fq_proc;
extern scheduler sche_proc;


void core_timer_enable_el1(){
	unsigned long tmp;
	asm volatile("mrs %0, cntkctl_el1" : "=r"(tmp));
	tmp |= 1;
	asm volatile("msr cntkctl_el1, %0" : : "r"(tmp));
}

void timer_time_after_booting()
{
	core_timer_enable();			// defined in timer.S
	timer_set_expired_time_by_sec(2);			// 2 seconds
}

void set_timeout(char *msg, char *arg2)
{
	//unsigned int duration;
	//duration = atoui(arg2);
	uart_puts("set_timeout() start\n");
	
	add_timer(timer_callback, msg, atoui(arg2));
}

void timer_callback(char *msg) 
{
	//uart_puts("timer_callback\n");
}

void timeout_event_init() 
{
	timeout_queue_head = 0;
	timeout_queue_tail = 0;
}


void add_timer(void (*callback)(char *), char *args, unsigned int duration)
{
/*
	unsigned int duration = 2;		// in seconds

	// queue be added first
	if(1)
	{
		//el1_interrupt_enable();
		core_timer_enable();			// defined in timer.S
		timer_set_expired_time_by_sec(duration);
	}
	else
	{
	
	}
	*/
	
	uart_puts("add_timer() start\n");
	
	// set timeout_event
	timeout_event *new_timeout_event =(timeout_event *)simple_alloc(sizeof(timeout_event));
	uart_puts("53\n");
	new_timeout_event->register_time = get_current_time();
		uart_puts("54\n");
	new_timeout_event->duration = duration;
		uart_puts("55\n");
	new_timeout_event->callback = callback;
	uart_puts("56\n");
	for (int i = 0; i < 20; i++) {
		new_timeout_event->args[i] = args[i];
		if (args[i] == '\0') break;
  	}
	new_timeout_event->prev = 0;
	new_timeout_event->next = 0;

	uart_puts("64\n");
	// add timeout_event into queue
	if (timeout_queue_head == 0) {					// queue is empty
		timeout_queue_head = new_timeout_event;
		timeout_queue_tail = new_timeout_event;
		uart_puts("69\n");
		core_timer_enable();
		timer_set_expired_time_by_sec(duration);
		uart_puts("72\n");
	} 
  	else {
		timeout_event *cur;
		unsigned long timeout = new_timeout_event->register_time + new_timeout_event->duration;
		// find position to add timeout_event
		for (cur = timeout_queue_head; cur; cur = cur->next) {
			if (cur->register_time + cur->duration > timeout) 
				break;
    	}

		// add timeout_event into queue
		if (cur == 0) {
			new_timeout_event->prev = timeout_queue_tail;
			timeout_queue_tail->next = new_timeout_event;
			timeout_queue_tail = new_timeout_event;
		} 
		else if (cur->prev == 0) {
			new_timeout_event->next = cur;
			timeout_queue_head->prev = new_timeout_event;
			timeout_queue_head = new_timeout_event;
			timer_set_expired_time_by_sec(duration);
		} 
		else {
			new_timeout_event->prev = cur->prev;
			new_timeout_event->next = cur;
			cur->prev->next = new_timeout_event;
			cur->prev = new_timeout_event;
		}
  	}
  	
  	uart_puts("finish: add_timer()\n");
}

void timer_set_expired_time_by_sec(unsigned int duration)
{
	unsigned long cntfrq_el0;
	asm volatile("mrs	%0, cntfrq_el0" : "=r"(cntfrq_el0));				// get frequency of the system counter
	asm volatile("msr	cntp_tval_el0 ,%0" :: "r"(cntfrq_el0 * duration));
}

void timer_set_expired_time_by_shift(unsigned int shift)
{
	unsigned long cntfrq_el0;
	asm volatile("mrs	%0, cntfrq_el0" : "=r"(cntfrq_el0));				// get frequency of the system counter
	asm volatile("msr	cntp_tval_el0 ,%0" :: "r"(cntfrq_el0>>shift));
}

/*
	CNTPCT_EL0 system register reports the current system count value
	CNTFRQ_EL0 reports the frequency of the system count
*/
unsigned long get_current_time() {
  unsigned long cntpct_el0, cntfrq_el0;
  asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct_el0));	
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq_el0));
  return cntpct_el0 / cntfrq_el0;
}


/********************************************************/
//                      irq								//
/********************************************************/

void timer_context_switch(struct trapframe *trapframe)
{
	Task *cur = sche_running_proc(&sche_proc);
	if (cur != 0)
	{		
		if (sche_proc.fork_queue->beg != 0)
		{
			// all fork proc in fork queue move to run queue
			sche_move_all_proc(TASK_FORK, TASK_RUN, &sche_proc);
		}

		sche_next(TASK_RUN, &sche_proc);
		Task *next = sche_running_proc(&sche_proc);
		if (next != cur)
		{
			proc_set_trapframe(&(next->trapframe), trapframe);  
		}
		else
		{

		}
	}
}

void timer_irq_el0(struct trapframe *trapframe){
	//uart_puts("( timer.c, timer_irq_el0000() )");

	//timer_context_switch(trapframe);
	
	//timer_set_expired_time_by_shift(5);
}

void timer_irq_el1(struct trapframe *trapframe){
	//uart_puts("( timer.c, timer_irq_el1() )");
	timer_context_switch(trapframe);

	/*
	else if (child != 0)
	{
		sche_pop_by_task(child, &sche_proc);
		parent->status = TASK_RUN;
		sche_push(child, &sche_proc);
    	proc_set_trapframe(&(child->trapframe), trapframe);  
	}
	*/
	//timer_set_expired_time_by_shift(5);
}

void timer_irq_el1_lab3(){
/*
  	uart_puts("el1 Current time: ");
  	uart_put_int(get_current_time());
  	uart_puts("");
	timer_set_expired_time_by_sec(2);*/
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
    	timer_set_expired_time_by_sec(next_duration);
  	} 
	else {
    	timeout_queue_head = 0;
    	timeout_queue_tail = 0;
    	core_timer_disable();
  	}
  	*/
  	
  	uart_puts("(timer_irq_el1) Current time: ");
  	uart_put_int(get_current_time());
  	timer_set_expired_time_by_sec(2);				// timer interrupt each 2 seconds

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
  	//kfree(timeout_queue_head);
  	if (next) {
    	next->prev = 0;
    	timeout_queue_head = next;
    	unsigned long next_duration = next->register_time + next->duration - get_current_time();
    	timer_set_expired_time_by_sec(next_duration);
  	} 
	else {
    	timeout_queue_head = 0;
    	timeout_queue_tail = 0;
    	core_timer_disable();
  	}
	  */
}

void timer_irq_el0_lab3(){	
  	uart_puts("(timer_irq_el0) Current time: ");
  	uart_put_int(get_current_time());
  	timer_set_expired_time_by_sec(2);				// timer interrupt each 2 seconds
}
