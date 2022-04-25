#ifndef _TASK_H
#define _TASK_H

enum task_status {
	TASK_IDLE = 0,
	TASK_RUN = 1,
	TASK_DEAD = 2
};

typedef struct _Task{
	unsigned long context[12+1+2+1+31];//kreg+ksp & spsr+elr & usp+ureg
	int id;
	enum task_status status;
	unsigned long a_addr,a_size,child;
	struct _Task* next;
	/*
	task stack:this ~ this+TASKSIZE
	*/
}Task;

#endif