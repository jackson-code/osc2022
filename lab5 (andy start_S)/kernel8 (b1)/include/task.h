#ifndef _TASK_H
#define _TASK_H

enum task_status {
	TASK_IDLE = 0,
	TASK_RUN = 1,
	TASK_DEAD = 2
};

// 13 element
struct register_context
{
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
    unsigned long fp;  // x29
    unsigned long lr;  // x30
    unsigned long sp;
};

typedef struct _Task{
    struct register_context reg;        // this item has to put on the first addr of the struct Task, this way make switching thread easy to get this item
	unsigned long context[2+1+31];      //spsr+elr & usp+ureg
	int id;
	enum task_status status;
	unsigned long a_size, child;// app addr, size, child process    
	struct _Task* next;

    unsigned long a_addr;
    unsigned long *code;                // user code
	/*
	task stack:this ~ this+TASKSIZE
	*/
}Task;








#endif