#include "thread.h"
#include "uart.h"
#include "mm.h"
#include "cpio.h"
#include "scheduler.h"

scheduler sche;
circular_queue rq;
circular_queue dq;
circular_queue fq;

static int task_id;

/*
 context switch
*/
void thread_schedule(){
	uart_puts("thread_schedule\n");

	Task *next_rq = sche_next(TASK_RUN, &sche);

	if(next_rq == 0) {					// run queue is empty, so switch to idle thread
		asm volatile("\
			mov x1, %0\n\
			bl thread_get_current\n\
			bl thread_switch\n\
			"::"r"(sche.idle));
	} else {
		asm volatile("\
			mov x1, %0\n\
			bl thread_get_current\n\
			bl thread_switch\n\
			"::"r"(next_rq));			// only use bl to avoid stack usage
	}
}

Task* thread_create(void* func){
	uart_puts("thead_create() begin\n");
	Task* new_task=(Task*)kmalloc(TASKSIZE);
	if((unsigned long)new_task % TASKSIZE){//aligned
		uart_puts("new_task isn't aligned!!\n");
		while(1){}
	}
	
	//new_task->reg.fp = (unsigned long)new_task + TASKSIZE;
	new_task->reg.lr = (unsigned long)func;
	//new_task->reg.sp = (unsigned long)new_task + TASKSIZE;

	new_task->id = task_id++;
	new_task->status = TASK_RUN;
	new_task->a_addr = new_task->a_size = 0;
	new_task->child = new_task->parent = new_task->next = (Task *)0;	
	
	uart_puts("thead_create() end\n");
	return new_task;
}

/*--------------------------------------------*/

void zombies_kill(){		//called by idle()
	Task* tar = 0;
	while ((tar = sche_pop(TASK_DEAD, &sche))) {
		kfree(tar);
	}
}

void idle(){
	uart_puts("idle()\n");

	// create idle thread
	sche.idle = (unsigned long)kmalloc(TASKSIZE);			// stored idle's reg
	thread_set_current(sche.idle);							// tpidr_el1 stored current thread info

	while(sche_next(TASK_RUN, &sche)){
		zombies_kill();
		thread_schedule();
	}

	uart_puts("idle(), kill all threads\n");
}

void delay(int period) {
    while (period--);
}

/*--------------------------------------------*/

int tid_get(){
	Task* cur = thread_get_current();
	return cur->id;
}

void exit(){
	uart_puts("exit(), id = ");

	Task* cur = thread_get_current();
	uart_put_int(cur->id);
	uart_puts("\n");

	sche_pop_by_task(cur, &sche);
	cur->status = TASK_DEAD;
	sche_push(cur, &sche);
	thread_schedule();
}


/*--------------------------------------------*/


void foo1(){
	for(int i = 0; i < 10; ++i){
		uart_puts("Thread id: ");
		uart_put_int(tid_get());
		uart_puts(", ");
		uart_put_int(i);
		uart_puts("\n");
		delay(1000000);
		thread_schedule();
	}
	exit();
}

void threadTest1(){
	sche_init(&rq, &dq, &fq, &sche);

	for(int i = 0; i < 3; ++i){
		Task *new_task = thread_create(foo1);
		sche_push(new_task, &sche);				// push into rq
	}
	idle();
	uart_puts("threadTest finish\n");
}