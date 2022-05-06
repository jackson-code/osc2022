#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "thread.h"
#include "uart.h"
#include "task.h"

typedef struct circular_queue {
	Task *beg;
    Task *end;
} circular_queue;

typedef struct scheduler {
	circular_queue *run_queue;
	circular_queue *dead_queue;
	circular_queue *fork_queue;
	unsigned long idle;
} scheduler;

void sche_init(circular_queue *rq, circular_queue *dq, circular_queue *fq, scheduler *sche);
void sche_push(Task *, scheduler *sche);
Task *sche_pop(enum task_status status, scheduler *sche);
Task *sche_pop_by_task( Task *tar, scheduler *sche);
Task *sche_pop_by_id(int id, scheduler *sche);
Task *sche_next(enum task_status status, scheduler *sche);


#endif