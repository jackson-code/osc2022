
#ifndef _THREAD_H
#define _THREAD_H

#include "uart.h"
#include "mm.h"
#include "cpio.h"
#include "scheduler.h"
#include "task.h"


#define TASKSIZE 4096
#define TASKFORK 2

extern Task *thread_get_current();
extern void thread_set_current();
extern void thread_switch();

Task* thread_create(void* func);
void threadSchedule();
int tidGet();
//void exec(char* path,char** argv);
void exit();
//int fork();
void threadTest1();
//void threadTest2();

#endif