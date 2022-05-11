#include "sys_call.h"
#include "process.h"
#include "mailbox.h"
#include "el1.h"
#include "exception.h"
//#include "my_print.h"

extern scheduler sche_proc;

void free_task(Task *tar);

void irq_enable() {
    asm volatile("msr daifclr, #2");
}

void irq_disable() {
    asm volatile("msr daifset, #2");
}

//void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
//void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }


void sys_get_task_id(struct trapframe* trapframe) {
    //unsigned long task_id = thread_get_current()->id;
    Task *cur = sche_running_proc(&sche_proc);
    trapframe->x[0] = cur->id;
}

void sys_uart_read(struct trapframe* trapframe) {
    char* buf = (char*) trapframe->x[0];
    unsigned long size = trapframe->x[1];
    
    int len = uart_get_string(buf, size);

    //irq_enable();
    //irq_disable();
    
    trapframe->x[0] = len;
}

void sys_uart_write(struct trapframe* trapframe) {
    const char* buf = (char*) trapframe->x[0];
    unsigned long size = trapframe->x[1];

    /*
    uart_puts("sys_uart_write()\nsize = ");
    uart_put_int(size);
    uart_puts("\n");
    uart_put_hex((unsigned int)buf);
    uart_puts("\n");
    */

    for (unsigned long i = 0; i < size; i++) {
        uart_send(buf[i]);
    }

    //irq_disable();
    trapframe->x[0] = size;
}

void sys_mbox_call(struct trapframe* trapframe) {
    unsigned char ch = (unsigned char) trapframe->x[0];
    unsigned int * mailbox = (unsigned int *)trapframe->x[1];

    int ret = mailbox_call(ch, mailbox);

    trapframe->x[0] = ret;
}

void sys_fork(struct trapframe* trapframe) {
    uart_puts("sys_fork()\n");
    Task *child = process_fork(trapframe);

    // parent move to fork queue
    Task *parent = sche_pop(TASK_RUN, &sche_proc); 
    parent->status = TASK_FORK;
	sche_push(parent, &sche_proc);

    // child push into run queue 
	sche_push(child, &sche_proc);          

    parent->trapframe.x[0] = child->id;     // parent return child id
    child->trapframe.x[0] = 0;              // child return 0

    // run child proc
    proc_set_trapframe(&(child->trapframe), trapframe);  
}

void sys_exit(struct trapframe* trapframe) {
    int status = trapframe->x[0];                   // useless, spec require

    uart_puts("sys_exit()\n");

	Task* cur = sche_pop(TASK_RUN, &sche_proc);
    
    Task *parent = cur->parent;
    Task *run = sche_next(TASK_RUN, &sche_proc);    // proc will run next
    Task *next = 0;
    
    //kfree(cur->code);
    free_task(cur);

    // run RQ, if RQ exist
    // run parent form FQ, if RQ empty
    // run other proc from FQ, if no parent
    if (run == 0)
    {
        uart_puts("\trun queue is empty, ");
        if (parent != 0)
        {
            uart_puts("pop parent from fork queue\n");
            parent->child = 0;                      // remove cur from parent
            sche_pop_by_task(parent, &sche_proc);  // remove parent from fork queue
            parent->status = TASK_RUN;
            sche_push(parent, &sche_proc);
            next = parent;
        } else {
            uart_puts("and no parent, pop other proc from fork queue\n");
            next = sche_pop(TASK_FORK, &sche_proc);
            if (next == 0)
            {
                uart_puts("\tfork queue is empty, stop\n");
                while (1) {}
            } else {
                next->status = TASK_RUN;
                sche_push(next, &sche_proc);
            }
        }
    } else {
        next = run;
    }
    uart_puts("\tupdate trapframe, continue to run another process\n");
    proc_set_trapframe(&(next->trapframe), trapframe);
}


/*
    The exec() functions return only if an error has occurred.  The
    return value is -1
*/
void sys_exec(struct trapframe* trapframe) {
    char *name = (char *)trapframe->x[0];
    char **argv = (char **)trapframe->x[1];

    //sys_exit(trapframe);

    el1_exec(name, argv);
    
    trapframe->x[0] = 0;
}

void sys_kill(struct trapframe* trapframe){
    int id = (int)trapframe->x[0];
    uart_puts("sys_kill()\n");

    Task *tar = sche_pop_by_id(id, &sche_proc);

    Task *child = tar->child;
    if (child)
    {
        child->parent = 0;
    }
    Task *parent = tar->parent;
    if (parent)
    {
        parent->child = 0;
    }
    
    //kfree(cur->code);
    free_task(tar);
}


void sys_uart_write_int(struct trapframe* trapframe) {
    const unsigned long buf = (unsigned long) trapframe->x[0];

    uart_put_int(buf);

    //irq_disable();
    trapframe->x[0] = 0;
}

void sys_uart_write_hex(struct trapframe* trapframe) {
    const unsigned int buf = (unsigned int ) trapframe->x[0];

    uart_put_hex(buf);

    //irq_disable();
    trapframe->x[0] = 0;
}


void sys_call_router(unsigned long sys_call_num, struct trapframe* trapframe) {
//    uart_puts("\nsys_call.c\n");
    switch (sys_call_num) {
        case SYS_GET_PID:
            sys_get_task_id(trapframe);
            break;

        case SYS_UART_READ:
            sys_uart_read(trapframe);
            break;

        case SYS_UART_WRITE:
            sys_uart_write(trapframe);
            break;
            
        case SYS_EXEC:
            sys_exec(trapframe);
            break;

        case SYS_FORK:
            sys_fork(trapframe);
            break;

        case SYS_EXIT:
            sys_exit(trapframe);
            break;

        case SYS_MBOX_CALL:
            sys_mbox_call(trapframe);
            break;

        case SYS_KILL:
            sys_kill(trapframe);
            break;


        case SYS_UART_WRITE_INT:
            sys_uart_write_int(trapframe);
            break;

        case SYS_UART_WRITE_HEX:
            sys_uart_write_hex(trapframe);
            break;        
    }
}


void free_task(Task *tar)
{
    if (tar != 0)
    {
        tar->parent = tar->child = tar->next = 0;
        kfree(tar);
    }
}