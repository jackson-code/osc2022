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
	unsigned long idx = 0;
	//uart_puts("\twalk() begin\n");
	for(int level = 3; level > 0; level--) {
		//page_table = (page_table_t)(((unsigned long)page_table) | KERNEL_VIRT_BASE);

		//pte_t *pte = &page_table[PT_IDX(level, va)];
		idx = PT_IDX(level, va);
		pte_t *pte = &page_table[idx];	// pte is current table's entry
		if(*pte & PD_TABLE) {	// page table and entry are exist
			page_table = (page_table_t)PTE2PA(*pte);
		} else {
			if(!alloc || (page_table = (pte_t *)kmalloc(PAGE_SIZE)) == 0)	// alloc for next level page table
				return 0;
			memset(page_table, 0, PAGE_SIZE);
			*pte = (unsigned long)page_table | PD_TABLE;			// entry of current level PD
			//*pte = (unsigned long)page_table | PD_TABLE | KERNEL_VIRT_BASE;			// entry of current level PD
			
			//*pte = PA2PTE(page_table) | PTE_V;
		}
		unsigned long for_debug = 0;
	}
	//uart_puts("\twalk() end\n");

	//page_table = (page_table_t)(((unsigned long)page_table) | KERNEL_VIRT_BASE);

	idx = PT_IDX(0, va);
	return &page_table[idx];	// return phys addr of entry in PTE
}

// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
unsigned long walkaddr(page_table_t page_table, unsigned long va)
{
	pte_t *pte;
	unsigned long pa;

	pte = walk(page_table, va, 0);
	if(pte == 0)
		return 0;
	pa = PTE2PA(*pte);
	return pa;
}

unsigned long *walk_pte_entry(page_table_t page_table, unsigned long va)
{
	pte_t *pte;

	pte = walk(page_table, va, 0);
	if(pte == 0)
		return 0;
	return pte;
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

typedef unsigned long uint64_t;
typedef char uint8_t;
#define PGD_SHIFT               39
#define PUD_SHIFT               30
#define PMD_SHIFT               21
#define PTE_SHIFT               12
#define PD_MASK                 0x1FFUL			// each index take 9 bits

uint64_t virtual_to_physical(uint64_t virt_addr) {
    return (virt_addr << 16) >> 16;
}

void* create_pgd(Task* task) {
    if (!task->mm.pgd) {
        void* page = kmalloc(PAGE_SIZE);		// vir addr
		memset(page, 0, PAGE_SIZE);
        if (page == NULL) return NULL;
        task->mm.pgd = virtual_to_physical((uint64_t)page);
    }
	uint64_t result = ((uint64_t)(task->mm.pgd)) | KERNEL_VIRT_BASE;
    return (void*)(result);
}

void* create_page_table(Task* task, uint64_t *table, uint64_t idx) {
    if (table == NULL) return NULL;
    if (!table[idx]) {
        void* page = kmalloc(PAGE_SIZE);		// virt addr
		memset(page, 0, PAGE_SIZE);
        if (page == NULL) return NULL;
		// ??: phys addr should shift left 12 bits, then or PD_table
        table[idx] = virtual_to_physical((uint64_t)page) | PD_TABLE;
    }
    return (void*)((table[idx] & ~0xFFF) | KERNEL_VIRT_BASE);
}

void* create_page_user(Task* task, uint64_t *pte, uint64_t idx, uint64_t user_addr) {
    if (pte == NULL) return NULL;
    if (!pte[idx]) {
        void* page = kmalloc(PAGE_SIZE);
		memset(page, 0, PAGE_SIZE);
        if (page == NULL) return NULL;
		// ??: should left shift 12 bits?
        pte[idx] = virtual_to_physical((uint64_t)page) | PTE_NORMAL_ATTR | PD_PERM_R | PD_PERM_KER_USER;
    }
    return (void*)((pte[idx] & ~0xFFF) | KERNEL_VIRT_BASE);
}

void* get_page_user(Task* task, uint64_t user_addr) {
    uint64_t pgd_idx = (user_addr & (PD_MASK << PGD_SHIFT)) >> PGD_SHIFT;
    uint64_t pud_idx = (user_addr & (PD_MASK << PUD_SHIFT)) >> PUD_SHIFT;
    uint64_t pmd_idx = (user_addr & (PD_MASK << PMD_SHIFT)) >> PMD_SHIFT;
    uint64_t pte_idx = (user_addr & (PD_MASK << PTE_SHIFT)) >> PTE_SHIFT;

	//
	// register 4 phys page for pgd, pud, pmd, pte
	//
	
	// virt addr in kernel space, create pgd in kernel space
    uint64_t* pgd = create_pgd(task);
	
	// virt addr in kernel space, create pud in kernel space, insert one entry into pgd
	uint64_t* pud = create_page_table(task, pgd, pgd_idx);	
	
	// virt addr in kernel space, create pmd in kernel space, insert one entry into pud
	uint64_t* pmd = create_page_table(task, pud, pud_idx);	
	
	// virt addr in kernel space, create pte in kernel space, insert one entry into pmd
    uint64_t* pte = create_page_table(task, pmd, pmd_idx);

	// insert one entry into pte, register 1 phys page for user_addr
    return create_page_user(task, pte, pte_idx, user_addr);
}



Task *yk_process_create(char *file_addr, unsigned long app_size) 
{
    Task *process = thread_create((void *)0);
	process->a_size = app_size;
	process->id = pid++;

    void* code_page = get_page_user(process, 0x0);					// virt
    void* stack_page = get_page_user(process, 0x0000ffffffffe000 - 8);	// virt

    // copy code to pc
    uint8_t* pc_ptr = (uint8_t*)code_page;
    uint8_t* code_ptr = (uint8_t*)file_addr;
    for (uint64_t i = 0; i < app_size; i++) { // copy code to virt addr
        *(pc_ptr + i) = *(code_ptr + i);
    }
	process->code = 0x0;	// virt

	process->reg.sp = 0x0000ffffffffe000 - 8;	// virt
	process->reg.fp = 0x0000ffffffffe000 - 8;	// virt

	return process;
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
	memset((unsigned long *)pa, 0, align_size);
	process->code = 0x0;	// virt

 	// buddy sys alloc an empty page table.
    //process->mm.pgd = 0;
	uart_puts("--- space for PGD ---\n");
    process->mm.pgd = page_table_create();

	uart_puts("--- space for PUD, PMD, PTE ---\n");
	unsigned long perm = PD_PERM_R | PD_PERM_KER_USER;
	int result = mappages(process->mm.pgd, process->code, align_size, pa, perm);
	if (result == 1)
		uart_puts("\tmappages() for user's code success\n");
	else 
		uart_puts("\tmappages() for user's code fail!!!\n");

	
	// alloc phys mem for user stack
	uart_puts("--- space for stack ---\n");
	unsigned long stack_size = PAGE_SIZE * 4;
	pa = (unsigned long)kmalloc(stack_size);
	memset((unsigned long *)pa, 0, stack_size);
	process->reg.sp = 0x0000fffffffff000 - 16;	// virt
	process->reg.fp = 0x0000fffffffff000 - 16;	// virt

	perm = PD_PERM_RW | PD_PERM_KER_USER;
	result = mappages(process->mm.pgd, 0x0000ffffffffb000, stack_size, pa, perm);
	if (result == 1)
		uart_puts("\tmappages() for user's stack success\n");
	else 
		uart_puts("\tmappages() for user's stack fail!!!\n");


	// have to moving user.img from cpio to memory(buddy system reserve for user.img)
	// otherwise can't get correct buf address of uartwrite
	uart_puts("\nmoving user prog...\n");
	pte_t *pte_code = walk_pte_entry(process->mm.pgd, process->code);
	unsigned long code_start =  (((*pte_code) & ~0xFFF) | KERNEL_VIRT_BASE);
	//unsigned long code_start =  (((*pte_code) & ~0xFFF));

	char *file = ((char *)((unsigned long)file_addr | KERNEL_VIRT_BASE));
	copy(file, (char *)code_start, app_size);

	//copy(file_addr, (char *)code_start, app_size);
	//copy(((unsigned long)file_addr) & (~0xffff000000000000), (char *)code_start, app_size);
	//copy(file_addr, (char *)process->code, app_size);

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

