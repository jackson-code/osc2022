#include "sys_call.h"
#include "process.h"
#include "mailbox.h"
#include "el1.h"
#include "exception.h"
#include "mm.h"
#include "vfs.h"
#include "scheduler.h"

extern scheduler sche_proc;

void free_task(Task *tar);


void sys_get_task_id(struct trapframe* trapframe) {
    Task *cur = sche_running_proc(&sche_proc);
    trapframe->x[0] = cur->id;
}

void sys_uart_read(struct trapframe* trapframe) {
    char* buf = (char*) trapframe->x[0];
    unsigned long size = trapframe->x[1];

    int len = uart_get_string(buf, size);
    
    trapframe->x[0] = len;
}

void sys_uart_write(struct trapframe* trapframe) {
    const char* buf = (char*) trapframe->x[0];
    unsigned long size = trapframe->x[1];

    for (unsigned long i = 0; i < size; i++) {
        uart_send(buf[i]);
    }

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
    //int status = trapframe->x[0];                   // useless, spec require

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

    trapframe->x[0] = 0;

    el1_exec(name, argv);
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
    
    free_task(tar);
}


void sys_uart_write_int(struct trapframe* trapframe) {
    const unsigned long buf = (unsigned long) trapframe->x[0];

    uart_put_int(buf);

    trapframe->x[0] = 0;
}

void sys_uart_write_hex(struct trapframe* trapframe) {
    const unsigned int buf = (unsigned int ) trapframe->x[0];

    uart_put_hex(buf);

    trapframe->x[0] = 0;
}

// syscall number : 11
void sys_open(struct trapframe* trapframe) {
    const char *pathname = (const char *)trapframe->x[0];
    int flags = (int)trapframe->x[1];

    uart_puts("--- sys_open ---\t");
    uart_puts(pathname);
    uart_puts("\n");

    Task *cur_task = sche_running_proc(&sche_proc);
    file_t *f = (file_t *)kmalloc(sizeof(file_t *));

    if (vfs_open(pathname, flags, &f) != 0) 
    {
        uart_puts("ERROR in sys_open():\t can't open the file's vnode\n");
        return;
    }
    
    // open regular file
    for (int i = FD_RESERVED; i < TASK_MAX_OPEN_FILES; i++)
    {
        if (cur_task->fd_table[i] == 0)
        {
            cur_task->fd_table[i] = f;
            trapframe->x[0] = i;            // return fd num
            return;
        }
    }
    uart_puts("ERROR in sys_open():\t process's fd_table is full\n");
    trapframe->x[0] = -1;
}

// syscall number : 12
void sys_close(struct trapframe* trapframe) {
    int fd = (int)trapframe->x[0];
    Task *cur_task = sche_running_proc(&sche_proc);

    uart_puts("--- sys_close ---\t");
    uart_puts(cur_task->fd_table[fd]->vnode->name);
    uart_puts("\n");

    int ret = vfs_close(cur_task->fd_table[fd]);
    cur_task->fd_table[fd] = 0;

    trapframe->x[0] = ret;                  // return success or not
}

// syscall number : 13
// remember to return read size or error code
void sys_write(struct trapframe* trapframe) {
    int fd = (int)trapframe->x[0];
    const void *buf = (const void *)trapframe->x[1];
    unsigned long len = (unsigned long)trapframe->x[2];

    Task *cur_task = sche_running_proc(&sche_proc);
    file_t *f = cur_task->fd_table[fd];

    uart_puts("--- sys_write ---\t");
    uart_puts(cur_task->fd_table[fd]->vnode->name);
    uart_puts("\n");

    trapframe->x[0] = vfs_write(f, buf, len);
}

// syscall number : 14
// remember to return read size or error code
void sys_read(struct trapframe* trapframe) {
    int fd = (int)trapframe->x[0];
    void *buf = (void *)trapframe->x[1];
    unsigned long len = (unsigned long)trapframe->x[2];

    Task *cur_task = sche_running_proc(&sche_proc);
    file_t *f = cur_task->fd_table[fd];

    uart_puts("--- sys_read ---\t");
    uart_puts(cur_task->fd_table[fd]->vnode->name);
    uart_puts("\n");

    trapframe->x[0] = vfs_read(f, buf, len);
}

// syscall number : 15
// you can ignore mode, since there is no access control
void sys_mkdir(struct trapframe* trapframe) {
    const char *pathname = (const char *)trapframe->x[0];
    
    uart_puts("--- sys_mkdir ---\t");
    uart_puts(pathname);
    uart_puts("\n");

    trapframe->x[0] =  vfs_mkdir(pathname);     // return success or not
}

// syscall number : 16
// you can ignore arguments other than target (where to mount) and filesystem (fs name)
void sys_mount(struct trapframe* trapframe) {
    //const char *src = 
    const char *target = (const char *)trapframe->x[0];
    const char *filesystem = (const char *)trapframe->x[1];
    //unsigned long flags = 
    //const void *data = 

    uart_puts("--- sys_mount ---\ttarget: ");
    uart_puts(target);
    uart_puts(", filesystem: ");
    uart_puts(filesystem);
    uart_puts("\n");

    trapframe->x[0] =  vfs_mount(target, filesystem);     // return success or not
}

// syscall number : 17
void sys_chdir(struct trapframe* trapframe) {
    const char *pathname = (const char *)trapframe->x[0];

    uart_puts("--- sys_chdir ---\ttarget: ");
    uart_puts(pathname);
    uart_puts("\n");

    trapframe->x[0] =  vfs_chdir(pathname);                  // return success or not
}

void sys_call_router(unsigned long sys_call_num, struct trapframe* trapframe) {
    //enable_interrupt();
    switch (sys_call_num) {
        case SYS_GET_PID:
            enable_interrupt();
            sys_get_task_id(trapframe);
            break;

        case SYS_UART_READ:
            enable_interrupt();
            sys_uart_read(trapframe);
            break;

        case SYS_UART_WRITE:
            enable_interrupt();
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
            enable_interrupt();
            sys_mbox_call(trapframe);
            break;

        case SYS_KILL:
            sys_kill(trapframe);
            break;


        case SYS_UART_WRITE_INT:
            enable_interrupt();
            sys_uart_write_int(trapframe);
            break;

        case SYS_UART_WRITE_HEX:
            enable_interrupt();
            sys_uart_write_hex(trapframe);
            break;    

        case SYS_OPEN:
            enable_interrupt();
            sys_open(trapframe);
            break;

        case SYS_CLOSE:
            enable_interrupt();
            sys_close(trapframe);
            break;

        case SYS_WRITE:
            enable_interrupt();
            sys_write(trapframe);
            break;

        case SYS_READ:
            enable_interrupt();
            sys_read(trapframe);
            break;

        case SYS_MKDIR:
            enable_interrupt();
            sys_mkdir(trapframe);
            break;

        case SYS_MOUNT:
            enable_interrupt();
            sys_mount(trapframe);
            break;

        case SYS_CHDIR:
            enable_interrupt();
            sys_chdir(trapframe);
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