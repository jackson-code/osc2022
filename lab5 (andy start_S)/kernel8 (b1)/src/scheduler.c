#include "scheduler.h"

//#define debug

void print_queue(scheduler *sche); // have bug
circular_queue *decide_queue(enum task_status status, scheduler *sche);

void sche_init(circular_queue *rq, circular_queue *dq, scheduler *sche)
{
    rq->beg = 0;
    rq->end = 0;

    dq->beg = 0;
    dq->end = 0;

    sche->run_queue = rq;
    sche->dead_queue = dq;
    sche->idle = 0;
}

void sche_push(Task *task, scheduler *sche)
{
    #ifdef debug
        uart_puts("sche_push() begin\n");
        print_queue(sche);
    #endif

    circular_queue *queue = decide_queue(task->status, sche);
    
    if (queue->beg == 0)                // empty queue
    {
        queue->beg = queue->end = task;
        task->next = task;
    }
    else                                // add to the end of the queue
    {
        queue->end->next = task;
        queue->end = task;
        queue->end->next = sche->run_queue->beg;
    }

    #ifdef debug
        print_queue(sche);
        uart_puts("sche_push() end\n");
    #endif
}

/*
 return and remove the first element in queue
 return 0 if no element in queue
*/ 
Task *sche_pop(enum task_status status, scheduler *sche)
{
    #ifdef debug
        uart_puts("sche_pop() begin\n");
        print_queue(sche);
    #endif

    circular_queue *queue = decide_queue(status, sche);

    if (queue->beg == 0)                // queue is empty
    {
        return 0;
    }
    else if (queue->beg == queue->end)  // only one element in queue
    {
        Task *ret = queue->beg;
        queue->beg = queue->end = 0;
        return ret;
    }
    else                                // more than one element in queue
    {
        Task *ret = queue->beg;
        queue->end->next = queue->beg->next;
        queue->beg = queue->beg->next;
        return ret;
    }

    #ifdef debug
        print_queue(sche);
        uart_puts("sche_pop() end\n");
    #endif
}

/*
 return and remove the specific element in queue
 return 0 if can't find the specific element in queue
*/ 
Task *sche_pop_specific(Task *tar, scheduler *sche)
{
    #ifdef debug
        uart_puts("sche_pop_specific() begin\n");
        print_queue(sche);
    #endif

    circular_queue *queue = decide_queue(tar->status, sche);

    if (queue->beg == 0)                // queue is empty
    {
        return 0;
    }
    else if (queue->beg == queue->end)  // only one element in queue
    {
        Task *ret = 0;
        if (queue->beg == tar) {
            ret = queue->beg;
            queue->beg = queue->end = 0;     
        }
        return ret;
    }
    else                                // more than one element in queue
    {
        Task *ret = queue->beg;
        Task *prev_ret = queue->end;
        do {
            if (ret == tar) {
                prev_ret->next = ret->next;
                //ret->next = 0;
                if (tar == queue->end) {            // end will be remove, so end must be updata
                    queue->end = queue->end->next;
                }
                queue->beg = queue->end->next;      // maybe beg will be remove, so beg must be updata
                
                #ifdef debug
                    print_queue(sche);
                    uart_puts("sche_pop_specific() end\n");
                #endif
                
                return ret;
            } else {                    // next element
                prev_ret = ret;
                ret = ret->next;
            }
        } while(ret != queue->beg);
        uart_puts("ERROR in scheduler.c, sche_pop_specific()\n");
        return 0;
    }
}

/*
 return the first element from rq
 return 0 if rq is empty
*/
Task *sche_next_rq(scheduler *sche)
{
    circular_queue *rq = sche->run_queue;
    if (rq->beg == 0)                // rq is empty
        return 0;
    else {
        Task *ret = rq->beg;
        rq->end = rq->beg;
        rq->beg = rq->beg->next;
        return ret;
    }
}

circular_queue *decide_queue(enum task_status status, scheduler *sche)
{
    if (status == TASK_RUN)
        return sche->run_queue;
    else if (status == TASK_DEAD)
        return sche->dead_queue;
    else {
        uart_puts("ERROR in scheduler.c, decide_task()\n");
        return 0;
    }
}

void print_queue(scheduler *sche)
{
    #ifdef debug
        Task *cur = sche->run_queue->beg; 
        uart_puts("\tRQ:");
        if (cur != 0)
        {
            do {
                uart_puts("\tid = ");
                uart_put_int(cur->id);
                uart_puts(", ");
                cur = cur->next;
            } while (cur != sche->run_queue->beg);
        }
        uart_puts("\n");

        cur = sche->dead_queue->beg; 
        uart_puts("\tDQ:");
        if (cur != 0)
        {
            do {
                uart_puts("\tid = ");
                uart_put_int(cur->id);
                uart_puts(", ");
                cur = cur->next;
            } while (cur != sche->dead_queue->beg);
        }
        uart_puts("\n");
    #endif
}