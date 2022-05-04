#include "process.h"


void copy(char *old, char *new, unsigned long size);
void update_trapframe(struct trapframe *old, struct trapframe *new);


void process_init()
{
	sche_init(&rq_proc, &dq_proc, &sche_proc);
}

Task *process_create(char *file_addr, unsigned long app_size) 
{
    Task *process = thread_create((void *)0);
	process->a_size = app_size;

	// alloc mem for user code section
	unsigned long page_count = ((app_size-1) / 4096) + 1;
	char *new_addr = kmalloc(page_count * 4096);
	process->code = (unsigned long *)new_addr;

	// have to moving user.img from cpio to memory(buddy system reserve for user.img)
	// otherwise can't get correct buf address of uartwrite
	uart_puts("\nmoving user prog...\n");
	/*
	while(app_size--){
		*new_addr++ = *file_addr++;
	}
	*/
	copy(file_addr, new_addr, app_size);

	return process;
}

Task *process_fork(struct trapframe *trapframe)
{
	Task *child = (Task *)kmalloc(4096);
	unsigned long *child_code = (unsigned long *)kmalloc(4096);
	child->code = child_code;

	//copy();
	//thread_get_current();

	Task *parent = sche_proc.run_queue->beg;		// beg is the current process
	update_trapframe(trapframe, &(parent->trapframe));

	// copy Task struct
	//unsigned long page_size = (((parent->a_size - 1) / 4096) + 1) * 4096;
	//copy((char *)parent, (char *)child, page_size);
	copy((char *)parent, (char *)child, 4096);

	// after copying Task to avoid be copy
	child->id = 33;
	parent->child = child;

	// copy code section
	copy((char *)parent->code, (char *)child->code, 4096);

	long long task_offset = parent - child;
	long long code_offset = parent->code - child->code;

	asm volatile("msr	spsr_el1, %0	\n"::"r"(trapframe->spsr_el1));
	// set stack pointer
	child->trapframe.sp_el0 = trapframe->sp_el0 + task_offset;
	asm volatile("msr	sp_el0, %0		\n"::"r"(child->trapframe.sp_el0));
	// set return address, return to the user.S
	child->trapframe.elr_el1 = trapframe->elr_el1 + code_offset;
	asm volatile("msr	elr_el1, %0		\n"::"r"(child->trapframe.elr_el1));

	return child;
}

void copy(char *old, char *new, unsigned long size)
{
	while(size--){
		*new++ = *old++;
	}
}

void update_trapframe(struct trapframe *new, struct trapframe *be_update)
{
	for (int i = 0; i < 31; i++)
	{
		be_update->x[i] = new->x[i];
	}
	be_update->elr_el1 = new->sp_el0;
	be_update->sp_el0 = new->sp_el0;
	be_update->spsr_el1 = new->spsr_el1;
}