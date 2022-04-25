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

circular_queue rq;
circular_queue dq;
scheduler sche;

void sche_init();
void sche_push(Task *);
Task *sche_pop(enum task_status status);
Task *sche_pop_specific( Task *tar);
Task *sche_next_rq();

#endif