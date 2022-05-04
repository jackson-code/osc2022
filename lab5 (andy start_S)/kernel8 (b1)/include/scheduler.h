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
	unsigned long idle;
} scheduler;

void sche_init(circular_queue *rq, circular_queue *dq, scheduler *sche);
//void sche_init(circular_queue rq, circular_queue dq, scheduler *sche);
//void sche_init(scheduler *sche);
void sche_push(Task *, scheduler *sche);
Task *sche_pop(enum task_status status, scheduler *sche);
Task *sche_pop_specific( Task *tar, scheduler *sche);
Task *sche_next_rq(scheduler *sche);

#endif