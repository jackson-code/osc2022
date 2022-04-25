
#include "uart.h"
#include "mm.h"
#include "cpio.h"
#ifndef __THREAD_H__
#define __THREAD_H__

#define TASKSIZE 4096
#define TASKEXIT 1
#define TASKFORK 2

typedef struct _Task{
	unsigned long context[12+1+2+1+31];//kreg+ksp & spsr+elr & usp+ureg
	int id;
	int status;
	unsigned long a_addr,a_size,child;
	struct _Task* next;
	/*
	task stack:this ~ this+TASKSIZE
	*/
}Task;

typedef struct{
	Task *beg,*end;
	unsigned long count;
	unsigned long caller;
}RQ;



int threadSchedule();
int tidGet();
void exec(char* path,char** argv);
void exit();
int fork();
void threadTest1();
void threadTest2();

#endif