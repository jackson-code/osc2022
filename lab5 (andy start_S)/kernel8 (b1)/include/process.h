#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "scheduler.h"
#include "trapframe.h"
#include "task.h"



circular_queue rq_proc;
circular_queue dq_proc;
scheduler sche_proc;

void process_init();
Task *process_create(char *file_addr, unsigned long file_size);
Task *process_fork(struct trapframe *trapframe);

#endif