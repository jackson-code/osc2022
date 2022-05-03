#include "sys_call.h"

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
    uart_get_string(buf, size);

    //irq_enable();
    for (unsigned long i = 0; i < size; i++) {
        //buf[i] = uart0_read();
    }
    buf[size] = '\0';
    //irq_disable();
    
    trapframe->x[0] = size;
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


/*
void sys_exec(struct trapframe* trapframe) {
    void (*func)() = (void(*)()) trapframe->x[0];
    do_exec(func);
    trapframe->x[0] = 0;
}

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

void sys_exit(struct trapframe* trapframe) {
    do_exit(trapframe->x[0]);
}

*/

void sys_uart_write_int(struct trapframe* trapframe) {
    const unsigned long buf = (unsigned long) trapframe->x[0];

    /*
    uart_puts("sys_uart_write()\nsize = ");
    uart_put_int(size);
    uart_puts("\n");
    uart_put_hex((unsigned int)buf);
    uart_puts("\n");
    */

        uart_put_int(buf);
    

    //irq_disable();
    trapframe->x[0] = 0;
}

void sys_uart_write_hex(struct trapframe* trapframe) {
    const unsigned int buf = (unsigned int ) trapframe->x[0];

    /*
    uart_puts("sys_uart_write()\nsize = ");
    uart_put_int(size);
    uart_puts("\n");
    uart_put_hex((unsigned int)buf);
    uart_puts("\n");
    */

        uart_put_hex(buf);

    //irq_disable();
    trapframe->x[0] = 0;
}





void sys_call_router(unsigned long sys_call_num, struct trapframe* trapframe) {
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

        case SYS_FORK:
            sys_fork(trapframe);
            break;

        case SYS_EXIT:
            sys_exit(trapframe);
            break;
            */


        case SYS_UART_WRITE_INT:
            sys_uart_write_int(trapframe);
            break;

        case SYS_UART_WRITE_HEX:
            sys_uart_write_hex(trapframe);
            break;        
    }
}