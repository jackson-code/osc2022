#ifndef _THREAD_H
#define _THREAD_H

#include "task.h"

#define TASKSIZE 4096

// functions in thread.S
extern Task *thread_get_current();
extern void thread_set_current();
extern void thread_switch();

Task* thread_create(void* func);
void thread_schedule();
int tid_get();
void exit();
void threadTest1();

#endif