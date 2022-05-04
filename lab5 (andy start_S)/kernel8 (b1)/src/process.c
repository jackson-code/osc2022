#include "task.h"
#include "process.h"
#include "scheduler.h"

circular_queue rq;
circular_queue dq;
scheduler sche;

void process_create(unsigned long app_size) 
{
    Task *process = thread_create((void *)0);
	process->a_size = app_size;
	sche.idle = process;
	thread_set_current(sche.idle);
	//asm volatile("msr tpidr_el1, %0\n"::"r"(sche.idle));	// tpidr_el1 stored current thread info

	// alloc mem for user code section
	unsigned long page_count = ((app_size-1) / 4096) + 1;
	char *new_addr = kmalloc(page_count * 4096);
	process->code = (unsigned long *)new_addr;
	// have to moving user.img from cpio to memory(buddy system reserve for user.img)
	// otherwise can't get correct buf address of uartwrite
	uart_puts("\nmoving user prog...\n");
	while(app_size--){
		//*new_addr++ = *file_addr++;
	}
}