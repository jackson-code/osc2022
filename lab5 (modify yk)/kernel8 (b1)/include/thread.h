#ifndef __THREAD_H__
#define __THREAD_H__

#include "uart.h"
#include "mm.h"
#include "cpio.h"

#define TASKSIZE 4096
#define TASKEXIT 1
#define TASKFORK 2

struct {
        // ARM calling convention
        // x0 - x18 can be overwritten by the called function
        unsigned long x19;
        unsigned long x20;
        unsigned long x21;
        unsigned long x22;
        unsigned long x23;
        unsigned long x24;
        unsigned long x25;
        unsigned long x26;
        unsigned long x27;
        unsigned long x28;
        unsigned long fp;       // x29
        unsigned long lr;       // x30
        unsigned long sp;
} register;

enum task_state {
    RUNNING,
    ZOMBIE,
    EXIT,

    FORK,
};

typedef struct _Task{
	struct register reg;
	int ddd;
	// 12 for x19~x28, lr, sp
	//unsigned long context[12+1+2+1+31];//kreg+ksp & spsr+elr & usp+ureg
	int id;
	enum task_state;
	//int status;
	unsigned long a_addr,a_size,child;
	struct _Task* next;
	/*
	task stack:this ~ this+TASKSIZE
	*/
}Task;

// Request Queue
typedef struct{
	Task *beg;	// executing thread
	Task *end;
}RQ;


extern void switch_to(struct register* prev, struct register* next);

void threadSchedule();
int tidGet();
void exec(char* path,char** argv);
void exit();
int fork();
void threadTest1();
void threadTest2();

#endif
