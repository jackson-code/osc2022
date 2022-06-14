#include "scheduler.h"
#include "thread.h"
#include "uart.h"

//#define debug

void print_queue(scheduler *sche); // have bug
circular_queue *decide_queue(enum task_status status, scheduler *sche);
Task *pop_by_id_in_queue(int id, circular_queue *queue);

void sche_init(circular_queue *rq, circular_queue *dq, circular_queue *fq, scheduler *sche)
{
    rq->beg = 0;
    rq->end = 0;

    dq->beg = 0;
    dq->end = 0;

    fq->beg = 0;
    fq->end = 0;

    sche->run_queue = rq;
    sche->dead_queue = dq;
    sche->fork_queue = fq;
    sche->idle = 0;
}

Task *sche_running_proc(scheduler *sche)
{
    return sche->run_queue->beg;
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
        queue->end->next = queue->beg;
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

    Task *ret = 0;

    circular_queue *queue = decide_queue(status, sche);

    if (queue->beg == 0)                // queue is empty
    {
        ret = 0;
    }
    else if (queue->beg == queue->end)  // only one element in queue
    {
        ret = queue->beg;
        queue->beg = queue->end = 0;
        //ret->next = 0;
        ret->next = ret;
    }
    else                                // more than one element in queue
    {
        ret = queue->beg;
        queue->end->next = queue->beg->next;
        queue->beg = queue->beg->next;
        //ret->next = 0;
        ret->next = ret;
    }

    #ifdef debug
        print_queue(sche);
        uart_puts("sche_pop() end\n");
    #endif

    return ret;
}

/*
 return and remove the specific element in queue
 return 0 if can't find the specific element in queue
*/ 
Task *sche_pop_by_task(Task *tar, scheduler *sche)
{
    #ifdef debug
        uart_puts("sche_pop_by_task() begin\n");
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
        ret->next = ret;
        return ret;
    }
    else                                // more than one element in queue
    {
        Task *ret = queue->beg;
        Task *prev_ret = queue->end;
        do {
            if (ret == tar) {
                prev_ret->next = ret->next;
                ret->next = ret;
                if (tar == queue->end) {            // end will be remove, so end must be updata
                    //queue->end = queue->end->next;
                    queue->end = prev_ret;
                }
                queue->beg = queue->end->next;      // maybe beg will be remove, so beg must be updata
                
                #ifdef debug
                    print_queue(sche);
                    uart_puts("sche_pop_by_task() end\n");
                #endif
                
                return ret;
            } else {                    // next element
                prev_ret = ret;
                ret = ret->next;
            }
        } while(ret != queue->beg);
        uart_puts("ERROR in scheduler.c, sche_pop_by_task()\n");
        return 0;
    }
}

/*
 return and remove the specific element in queue
 return 0 if can't find the specific element in queue
*/ 
Task *sche_pop_by_id(int id, scheduler *sche)
{
    #ifdef debug
        uart_puts("sche_pop_by_id() begin\n");
        print_queue(sche);
    #endif

    Task *ret = 0;
    if ((ret = pop_by_id_in_queue(id, sche->run_queue)) != 0)
    {
        uart_puts("\tfind in RQ\n");
    }else if ((ret = pop_by_id_in_queue(id, sche->fork_queue)) != 0) {
        uart_puts("\tfind in FQ\n");
    }else if ((ret = pop_by_id_in_queue(id, sche->dead_queue)) != 0) {
        uart_puts("\tfind in DQ\n");
    }else {
        uart_puts("\tpid = ");
        uart_put_int(id);
        uart_puts(" can't find\n");
    }
    
    #ifdef debug
        print_queue(sche);
        uart_puts("sche_pop_by_id() end\n");
    #endif

    return ret;
}

Task *pop_by_id_in_queue(int id, circular_queue *queue)
{
    if (queue->beg == 0)                // queue is empty
    {
        return 0;
    }
    else if (queue->beg == queue->end)  // only one element in queue
    {
        Task *ret = 0;
        if (queue->beg->id == id) {
            ret = queue->beg;
            queue->beg = queue->end = 0;     
        }
        ret->next = ret;
        return ret;
    }
    else                                // more than one element in queue
    {
        Task *ret = queue->beg;
        Task *prev_ret = queue->end;
        do {
            if (ret->id == id) {
                prev_ret->next = ret->next;
                ret->next = ret;
                if (ret == queue->end) {            // end will be remove, so end must be updata
                    //queue->end = queue->end->next;
                    queue->end = prev_ret;
                }
                queue->beg = queue->end->next;      // maybe beg will be remove, so beg must be updata
                
                //#ifdef debug
                //    print_queue(sche);
                //    uart_puts("sche_pop_by_id() end\n");
                //#endif
                
                return ret;
            } else {                    // next element
                prev_ret = ret;
                ret = ret->next;
            }
        } while(ret != queue->beg);
        //uart_puts("ERROR in scheduler.c, pop_by_id_in_queue()\n");
        return 0;
    }
}

/*
 return the first element, and move it to the end
 return 0 if rq is empty
*/
Task *sche_next(enum task_status status, scheduler *sche)
{
    circular_queue *queue = decide_queue(status, sche);

    if (queue->beg == 0)                // rq is empty
        return 0;
    else {
        Task *ret = queue->beg;
        queue->end = queue->beg;
        queue->beg = queue->beg->next;
        return ret;
    }
}

circular_queue *decide_queue(enum task_status status, scheduler *sche)
{
    if (status == TASK_RUN)
        return sche->run_queue;
    else if (status == TASK_DEAD)
        return sche->dead_queue;
    else if (status == TASK_FORK)
        return sche->fork_queue;
    else {
        uart_puts("ERROR in scheduler.c, decide_task()\n");
        return 0;
    }
}

/*
    all proc in from_queue move to to_queue
    //return 1 if success, 
    //return 0 if fail
*/
void sche_move_all_proc(enum task_status from_status, enum task_status to_status, scheduler *sche)
{
    if (from_status == to_status)
    {
        return;
    }
   
    circular_queue *from_q = decide_queue(from_status, sche);
    Task *moving = 0;
    while (from_q->beg != 0)
    {
        moving = sche_pop(from_status, sche);
        moving->status = to_status;
        sche_push(moving, sche);
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

        cur = sche->fork_queue->beg; 
        uart_puts("\tFQ:");
        if (cur != 0)
        {
            do {
                uart_puts("\tid = ");
                uart_put_int(cur->id);
                uart_puts(", ");
                cur = cur->next;
            } while (cur != sche->fork_queue->beg);
        }
        uart_puts("\n");
    #endif
}