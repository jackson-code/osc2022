#include "process.h"
#include "my_string.h"
#include "mm.h"
#include "uart.h"
#include "thread.h"
#include "vm.h"
#include "utils.h"

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




/*
	PGD: level 3
	PUD: level 2
	PMD: level 1
	PTE: level 0
	
	Params:
		page_table: phys addr of PGD
*/
// Return the address of the PTE in page table pagetable*
// that corresponds to virtual address va.  If alloc!=0,*
// create any required page-table pages.*
//*
// The risc-v Sv39 scheme has three levels of page-table*
// pages. A page-table page contains 512 64-bit PTEs.*
// A 64-bit virtual address is split into five fields:*
//   39..63 — must be zero.*
//   30..38 — 9 bits of level-2 index.*
//   21..39 — 9 bits of level-1 index.*
//   12..20 — 9 bits of level-0 index.*
//    0..12 — 12 bits of byte offset within the page.*
pte_t *walk(page_table_t page_table, unsigned long va, int alloc)
{
	//uart_puts("\twalk() begin\n");
	for(int level = 3; level > 0; level--) {
		//pte_t *pte = &page_table[PT_IDX(level, va)];
		unsigned long idx = PT_IDX(level, va);
		pte_t *pte = &page_table[idx];
		if(*pte & PD_TABLE) {							// page table and entry are exist
			page_table = (page_table_t)PTE2PA(*pte);
		} else {
			if(!alloc || (page_table = (pte_t *)kmalloc(PAGE_SIZE)) == 0)	// alloc for next level page table
				return 0;
			memset(page_table, 0, PAGE_SIZE);
			*pte = (unsigned long)page_table | PD_TABLE;
			
			//*pte = PA2PTE(page_table) | PTE_V;
		}
		unsigned long for_debug = 0;
	}
	//uart_puts("\twalk() end\n");
	return &page_table[PT_IDX(0, va)];


/*
   for (int level = 3; level > 0; level--) {
    pte_t *pte_t = ...;
    if (*pte_t is a entry) {
      pagetable = *pte_t;
    } else {
      pagetable = alloc_page();
      memset(pagetable, 0, 0x1000);
      *pte_t = ...;
    }
  }
  return ...;
  */
}

// pa: phys addr alloc by buddy
// Create PTEs for virtual addresses starting at va that refer to*
// physical addresses starting at pa. va and size might not*
// be page-aligned. Returns 1 on success, -1 if walk() couldn’t*
// allocate a needed page-table page.*
int mappages(page_table_t page_table, unsigned long va, unsigned long align_size, unsigned long pa, unsigned long perm)
{
	unsigned long va_end;
	pte_t *pte;

//	va_end = va + align_size;
	va_end = va + align_size - PAGE_SIZE;
	for(;;){
		if((pte = walk(page_table, va, 1)) == 0)
			return -1;
		/* don't know what is it...
		if(*pte_t & PTE_V)
			panic("remap");
			*/
		*pte = PA2PTE(pa) | perm | PTE_NORMAL_ATTR;		// add phys addr, attr into pte
		
		if(va == va_end)
			break;
		va += PAGE_SIZE;
		pa += PAGE_SIZE;
	}
	return 1;
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

	// alloc phys mem for user code section
	unsigned long align_size = page_amount_round_up(app_size) * PAGE_SIZE;
	uart_puts("--- space for code section ---\n");
	unsigned long pa = (unsigned long)kmalloc(align_size);
	process->code = 0x0;	// virt

 	// buddy sys alloc an empty page table.
    //process->mm.pgd = 0;
	uart_puts("--- space for PGD ---\n");
    process->mm.pgd = page_table_create();

	uart_puts("--- space for PUD, PMD, PTE ---\n");
	unsigned long perm = PD_PERM_RW | PD_PERM_KER_USER;
	int result = mappages(process->mm.pgd, process->code, align_size, pa, perm);
	if (result == 1)
		uart_puts("\tmappages() for user's code success\n");
	else 
		uart_puts("\tmappages() for user's code fail!!!\n");

	
	// alloc phys mem for user stack
	uart_puts("--- space for stack ---\n");
	unsigned long stack_size = PAGE_SIZE * 4;
	pa = (unsigned long)kmalloc(stack_size);
	process->reg.sp = 0xfffffffff000;	// virt
	process->reg.fp = 0xfffffffff000;	// virt

 	// buddy sys alloc an empty page table.
    //process->mm.pgd = 0;
	/*
	uart_puts("--- space for PGD ---\n");
    process->mm.pgd = page_table_create();

	uart_puts("--- space for PUD, PMD, PTE ---\n");
	perm = PD_PERM_RW | PD_PERM_KER_USER;
	*/
	result = mappages(process->mm.pgd, 0xffffffffb000, stack_size, pa, perm);
	if (result == 1)
		uart_puts("\tmappages() for user's stack success\n");
	else 
		uart_puts("\tmappages() for user's stack fail!!!\n");


	// have to moving user.img from cpio to memory(buddy system reserve for user.img)
	// otherwise can't get correct buf address of uartwrite
	uart_puts("\nmoving user prog...\n");
	copy(file_addr, (char *)process->code, app_size);

	return process;
}

Task *process_fork(struct trapframe *trapframe)
{
	Task *child = (Task *)kmalloc(4096);

	Task *parent = sche_proc.run_queue->beg;		// beg is the current process
	proc_set_trapframe(trapframe, &(parent->trapframe));		// store parent info, for continue to run after child exit

	// copy Task struct
	copy((char *)parent, (char *)child, 4096);

	// after copying Task to avoid be copy
	child->id = pid++;
	parent->child = child;
	child->parent = parent;
	child->child = (Task *)0;	

	// copy code section
	//copy((char *)parent->code, (char *)child->code, 4096);

	unsigned long task_offset = parent->trapframe.sp_el0 - (unsigned long)parent;
	child->trapframe.sp_el0 = (unsigned long)child + task_offset;
	
	return child;
}

void copy(char *old, char *new, unsigned long size)
{
	while(size--){
		*new++ = *old++;
	}
}

