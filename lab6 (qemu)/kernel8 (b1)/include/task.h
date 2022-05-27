#ifndef _TASK_H
#define _TASK_H

#include "trapframe.h"

#define MAX_USER_PAGES      16
#define MAX_KERNEL_PAGES    16

typedef unsigned long *page_table_t;
typedef unsigned long pte_t;

typedef struct mm {
    page_table_t pgd;							// phys?
    unsigned long user_pages_count;
    unsigned long user_pages[MAX_USER_PAGES];
    unsigned long kernel_pages_count;
    unsigned long kernel_pages[MAX_KERNEL_PAGES];	// virt
}mm_t;            

enum task_status {
	TASK_IDLE = 0,
	TASK_RUN =  1,
	TASK_DEAD = 2,
    TASK_FORK = 3,
};

// only used in thread, no use in process
struct register_context
{
    // ARM calling convention
    // x0 - x18 can be overwritten by the called function
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;  // x29
    unsigned long lr;  // x30
    unsigned long sp;
};

typedef struct _Task{
    struct register_context reg;        // this item has to put on the first addr of the struct Task, this way make switching thread easy to get this item
	int id;
	enum task_status status;
    struct _Task *child;                // child process    
    struct _Task *parent;               // parent process    
	struct _Task *next;

    struct trapframe trapframe;         // app register & sp_el0, spsr_el1, elr_el1
    
	unsigned long a_size;               // app size
    unsigned long a_addr;               // app address
    unsigned long code;                // app code
    
    mm_t mm;
    

	/*
	task stack:this ~ this+TASKSIZE(sp point to here in begining)
	*/
}Task;








#endif