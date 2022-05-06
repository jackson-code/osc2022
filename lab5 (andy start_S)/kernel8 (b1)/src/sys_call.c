#include "sys_call.h"
#include "process.h"
//#include "my_print.h"

//extern circular_queue rq_proc;
//extern circular_queue dq_proc;
extern scheduler sche_proc;


void irq_enable() {
    asm volatile("msr daifclr, #2");
}

void irq_disable() {
    asm volatile("msr daifset, #2");
}
//void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
//void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }


void sys_get_task_id(struct trapframe* trapframe) {
    unsigned long task_id = thread_get_current()->id;
    trapframe->x[0] = task_id;
}



void sys_uart_read(struct trapframe* trapframe) {
    char* buf = (char*) trapframe->x[0];
    unsigned long size = trapframe->x[1];
    
    //uart_gets((char*)x0,(int)x1,1);
    int len = uart_get_string(buf, size);

    //irq_enable();
    for (unsigned long i = 0; i < size; i++) {
        //buf[i] = uart0_read();
    }
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



void sys_exec(struct trapframe* trapframe) {
    void (*func)() = (void(*)()) trapframe->x[0];
    //do_exec(func);
    trapframe->x[0] = 0;
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

    // parent
    parent->trapframe.x[0] = child->id;

    // child
    child->trapframe.x[0] = 0;

    // run child proc
    proc_set_trapframe(&(child->trapframe), trapframe);  
}


/*
void sys_fork(struct trapframe* trapframe) {
    struct task_t* parent_task = get_current_task();

    int child_id = privilege_task_create(return_from_fork, parent_task->priority);
    struct task_t* child_task = &task_pool[child_id];

    char* child_kstack = &kstack_pool[child_task->id][KSTACK_TOP_IDX];
    char* parent_kstack = &kstack_pool[parent_task->id][KSTACK_TOP_IDX];
    char* child_ustack = &ustack_pool[child_task->id][USTACK_TOP_IDX];
    char* parent_ustack = &ustack_pool[parent_task->id][USTACK_TOP_IDX];

    uint64_t kstack_offset = parent_kstack - (char*)trapframe;
    uint64_t ustack_offset = parent_ustack - (char*)trapframe->sp_el0;

    for (uint64_t i = 0; i < kstack_offset; i++) {
        *(child_kstack - i) = *(parent_kstack - i);
    }
    for (uint64_t i = 0; i < ustack_offset; i++) {
        *(child_ustack - i) = *(parent_ustack - i);
    }

    // place child's kernel stack to right place
    child_task->cpu_context.sp = (uint64_t)child_kstack - kstack_offset;

    // place child's user stack to right place
    struct trapframe* child_trapframe = (struct trapframe*) child_task->cpu_context.sp;
    child_trapframe->sp_el0 = (uint64_t)child_ustack - ustack_offset;

    child_trapframe->x[0] = 0;
    trapframe->x[0] = child_task->id;
}
*/

void sys_exit(struct trapframe* trapframe) {
    int status = trapframe->x[0];                   // useless, spec require

    uart_puts("sys_exit()\n");

	Task* cur = sche_pop(TASK_RUN, &sche_proc);
    
    Task *parent = cur->parent;
    Task *run = sche_next(TASK_RUN, &sche_proc);    // proc will run next
    Task *next = 0;
    
    //kfree(cur->code);
    kfree(cur);
    
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
            sche_pop_specific(parent, &sche_proc);  // remove parent from fork queue
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
            /*
        case SYS_EXEC:
            sys_exec(trapframe);
            break;
*/
        case SYS_FORK:
            sys_fork(trapframe);
            break;

        case SYS_EXIT:
            sys_exit(trapframe);
            break;


        case SYS_UART_WRITE_INT:
            sys_uart_write_int(trapframe);
            break;

        case SYS_UART_WRITE_HEX:
            sys_uart_write_hex(trapframe);
            break;        
    }
}


