#include "process.h"
//#include "my_print.h"
#include "my_string.h"


int pid = 0;

void copy(char *old, char *new, unsigned long size);


void print_reg(char *reg_name)
{
	unsigned int reg_value = 0;
    if (str_cmp("elr_el1", reg_name) == 0)
	{
		uart_puts("\t\telr_el1 = 0x");		
		asm volatile("mrs	%0, elr_el1		\n"
					:"=r"(reg_value));
		uart_put_hex(reg_value);
		uart_puts("\n");
	}
	else {
		uart_puts("\t\tERROR in my_print.c, print_reg() : unsupport register name\n");		
	}
}

void proc_set_trapframe(struct trapframe *new, struct trapframe *tf)
{
	/*
	for (int i = 0; i < 31; i++)
	{
		tf->x[i] = new->x[i];
	}
	tf->elr_el1 = new->sp_el0;
	tf->sp_el0 = new->sp_el0;
	tf->spsr_el1 = new->spsr_el1;
*/

	for (int i = 0; i < 31; i++)
    {
        asm("str %0, [%1, %2]"
            :
            :"r" (new->x[i]), "r" (tf), "r" (8 * i));
    }
    

    asm("str %0, [%1, #8 * 31]\n"
        :
        :"r" (new->sp_el0), "r" (tf));
    
    asm("str %0, [%1, #8 * 32]\n"
        :
        :"r" (new->elr_el1), "r" (tf));

    asm("str %0, [%1, #8 * 33]\n"
        :
        :"r" (new->spsr_el1), "r" (tf));
    
}

void proc_get_trapframe(struct trapframe *old, struct trapframe *tf)
{
	
	for (int i = 0; i < 31; i++)
	{
		old->x[i] = tf->x[i];
	}
	old->elr_el1 = tf->sp_el0;
	old->sp_el0 = tf->sp_el0;
	old->spsr_el1 = tf->spsr_el1;


/*
	for (int i = 0; i < 31; i++)
    {
        asm("ldr %0, [%1, %2]"
            :
            :"r" (old->x[i]), "r" (tf), "r" (8 * i));
    }
    

    asm("ldr %0, [%1, #8 * 31]\n"
        :
        :"r" (old->sp_el0), "r" (tf));
    
    asm("ldr %0, [%1, #8 * 32]\n"
        :
        :"r" (old->elr_el1), "r" (tf));

    asm("ldr %0, [%1, #8 * 33]\n"
        :
        :"r" (old->spsr_el1), "r" (tf));
    */
}

void process_init()
{
	sche_init(&rq_proc, &dq_proc, &fq_proc, &sche_proc);
}

Task *process_create(char *file_addr, unsigned long app_size) 
{
    Task *process = thread_create((void *)0);
	process->a_size = app_size;
	process->id = pid++;

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
	//unsigned long *child_code = (unsigned long *)kmalloc(4096);

	//copy();
	//thread_get_current();

	//print_reg("elr_el1");

	Task *parent = sche_proc.run_queue->beg;		// beg is the current process
	//proc_get_trapframe(&(parent->trapframe), trapframe);		// store parent info, for continue to run after child exit
	proc_set_trapframe(trapframe, &(parent->trapframe));		// store parent info, for continue to run after child exit

	//print_reg("elr_el1");


	// copy Task struct
	//unsigned long page_size = (((parent->a_size - 1) / 4096) + 1) * 4096;
	//copy((char *)parent, (char *)child, page_size);
	copy((char *)parent, (char *)child, 4096);

	// after copying Task to avoid be copy
	//child->id = 33;
	child->id = pid++;
	parent->child = child;
	child->parent = parent;
	//child->code = child_code;

	// copy code section
	//copy((char *)parent->code, (char *)child->code, 4096);

	unsigned long task_offset = parent->trapframe.sp_el0 - (unsigned long)parent;
	child->trapframe.sp_el0 = (unsigned long)child + task_offset;
	
	//unsigned long code_offset = trapframe->elr_el1 - (unsigned long)(parent->code);
	//child->trapframe.elr_el1 = (unsigned long)child->code + code_offset;

	return child;
}

void copy(char *old, char *new, unsigned long size)
{
	while(size--){
		*new++ = *old++;
	}
}

