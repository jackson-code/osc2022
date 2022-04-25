
#ifndef _THREAD_H
#define _THREAD_H

#include "uart.h"
#include "mm.h"
#include "cpio.h"
#include "scheduler.h"
#include "task.h"


#define TASKSIZE 4096
#define TASKEXIT 1
#define TASKFORK 2





void threadSchedule();
int tidGet();
void exec(char* path,char** argv);
void exit();
int fork();
void threadTest1();
void threadTest2();

#endif