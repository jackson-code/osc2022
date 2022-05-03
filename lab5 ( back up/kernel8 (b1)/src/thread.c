#include "thread.h"

extern scheduler sche;
static int task_cnter;

void threadSwitch(){
	asm volatile("\
_threadSwitch:\n\
		stp x19, x20, [x0, 16 * 0]\n\
		stp x21, x22, [x0, 16 * 1]\n\
		stp x23, x24, [x0, 16 * 2]\n\
		stp x25, x26, [x0, 16 * 3]\n\
		stp x27, x28, [x0, 16 * 4]\n\
		stp x29, x30, [x0, 16 * 5]\n\
		mov x9, sp\n\
		str x9, [x0, 16 * 6]\n\
		\n\
		ldp x19, x20, [x1, 16 * 0]\n\
		ldp x21, x22, [x1, 16 * 1]\n\
		ldp x23, x24, [x1, 16 * 2]\n\
		ldp x25, x26, [x1, 16 * 3]\n\
		ldp x27, x28, [x1, 16 * 4]\n\
		ldp x29, x30, [x1, 16 * 5]\n\
		ldr x9, [x1, 16 * 6]\n\
		mov sp, x9\n\
		\n\
		msr tpidr_el1, x1\n\
		\n\
		ret\n\
	"::);
}


/*
 context switch
*/
void threadSchedule(){
	uart_puts("thread_schedule\n");

	Task *next_rq = sche_next_rq();

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
			"::"r"(next_rq));			//only use bl to avoid stack usage
	}
}

Task* thread_create(void* func){
	Task* new_task=(Task*)kmalloc(TASKSIZE);
	if((unsigned long)new_task%TASKSIZE){//aligned
		uart_puts("new_task isn't aligned!!\n");
		while(1){}
	}
	
	uart_puts("thead_create, assign fp, lr, sp, \t");
	new_task->reg.fp = (unsigned long)new_task + TASKSIZE;
	new_task->reg.lr = (unsigned long)func;
	new_task->reg.sp = (unsigned long)new_task + TASKSIZE;

	new_task->id = task_cnter++;
	new_task->status = TASK_RUN;
	new_task->a_addr = new_task->a_size = new_task->child=0;
	new_task->next = 0;

	sche_push(new_task);		// push into rq

	return new_task;
}

/*--------------------------------------------*/

void zombiesKill(){//called by idle()
	Task* tar = 0;
	while ((tar = sche_pop(TASK_DEAD))) {
		kfree(tar);
	}
}

void taskUpdate(Task* p,Task* c){
	p->status^=TASKFORK;
	p->child=c->id;

	Task* tmp=c->next;
	char* src=(char*)p;
	char* dst=(char*)c;
	for(int i=0;i<TASKSIZE;++i){//task copy
		*dst=*src;
		dst++;
		src++;
	}

	c->id=p->child;
	uart_puts("Please enter app load address (Hex): ");
	//c->a_addr=uart_get_hex(1);
	c->child=0;
	c->next=tmp;

	long k_delta=(long)c-(long)p;
	long a_delta=(long)c->a_addr-(long)p->a_addr;
	c->reg.fp += k_delta;//kernel fp
	c->reg.sp += k_delta;//kernel sp

	c->context[14]+=a_delta;//elr_el1
	c->context[15]+=a_delta;//sp_el0
	c->context[45]+=a_delta;//user fp
	c->context[46]+=a_delta;//user lr

	src=(char*)(p->context[15]);
	dst=(char*)(c->context[15]);
	for(int i=0,ii=p->a_addr+p->a_size-(p->context[15]);i<ii;++i){//program copy
		*dst=*src;
		dst++;
		src++;
	}
}

void doFork(){//called by idle()
	Task* tar=rq.beg->next;
	while(tar){
		if((tar->status)&TASKFORK){
			Task* child=thread_create(0);
			taskUpdate(tar,child);
		}
		tar=tar->next;
	}
}

void idle(){
	uart_puts("idle()\n");

	// create idle thread
	sche.idle = (unsigned long)kmalloc(TASKSIZE);			// stored idle's reg
	asm volatile("msr tpidr_el1, %0\n"::"r"(sche.idle));	// tpidr_el1 stored current thread info

	while(sche_next_rq()){
		//uart_getc();
		zombiesKill();
		//doFork();
		threadSchedule();
	}

	uart_puts("idle(), kill all threads\n");
}
void delay(int period) {
    while (period--);
}

/*--------------------------------------------*/

int tidGet(){
	Task* cur;
	asm volatile("mrs %0, tpidr_el1\n":"=r"(cur):);
	return cur->id;
}

void exec(char* path,char** argv){//will not reset sp...
	unsigned long a_addr;
	uart_puts("Please enter app load address (Hex): ");
	//a_addr=uart_get_hex(1);
	//loadApp_with_argv(path,a_addr,argv,&(rq.beg->a_addr),&(rq.beg->a_size));
	exit();
}

void exit(){
	uart_puts("exit(), id = ");

	Task* cur;
	asm volatile("mrs %0, tpidr_el1\n":"=r"(cur):);
	uart_put_int(cur->id);
	uart_puts("\n");

	sche_pop_specific(cur);
	cur->status = TASK_DEAD;
	sche_push(cur);
	threadSchedule();

/*
	while(1){
		uart_puts("exit() failed!!\n");
	}
	*/
}

int fork(){
	rq.beg->status|=TASKFORK;
	threadSchedule();
	return rq.beg->child;
}

/*--------------------------------------------*/


void foo1(){
	for(int i = 0; i < 10; ++i){
		uart_puts("Thread id: ");
		uart_put_int(tidGet());
		uart_puts(", ");
		uart_put_int(i);
		uart_puts("\n");
		//uart_getc();
		delay(1000000);
		threadSchedule();
	}
	exit();
}

void threadTest1(){
	for(int i = 0; i < 3; ++i){
		thread_create(foo1);
	}
	idle();
	uart_puts("threadTest finish\n");
}

void foo2(){
	char* argv[]={"argv_test","-o","arg2",0};
	exec("app1",argv);
}

void threadTest2(){
	Task* cur=thread_create(0);//use startup stack (not kernel stack)
	asm volatile("msr tpidr_el1, %0\n"::"r"((unsigned long)cur));//TODO

	thread_create(foo2);

	idle();
}
