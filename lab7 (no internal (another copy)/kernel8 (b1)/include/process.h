#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "scheduler.h"
#include "trapframe.h"
#include "task.h"

circular_queue rq_proc;
circular_queue dq_proc;
circular_queue fq_proc;
scheduler sche_proc;

void print_reg(char *reg_name);

void process_init();
Task *process_create(char *file_addr, unsigned long file_size);
Task *process_fork(struct trapframe *trapframe);
void proc_set_trapframe(struct trapframe *new, struct trapframe *tf);
void proc_get_trapframe(struct trapframe *old, struct trapframe *tf);

#endif