#include "vm.h"
#include "mm.h"
#include "utils.h"

/*
	size in bytes.
	reutrn round up page amount
	e.g. 
	size = 4095, return 1
	size = 4097, return 2
*/
unsigned long page_amount_round_up(unsigned long size){
	return (((size-1) / PAGE_SIZE) + 1);
}

// create an empty user page table.
// return phys addr of page table allocated by buddy sys
// return 0 if out of memory.
page_table_t page_table_create()
{
    /*
    // calculate amount of entry
    unsigned long pmd_amount = align_size / PMD_GRANULE;
    if (pmd_amount == 0)
    {
        pmd_amount = 1;
    }
    unsigned long pte_amount = align_size / PTE_GRANULE;
    if (pte_amount == 0)
    {
        pte_amount = 1;
    }
    unsigned long entry_amount = 1 + pmd_amount + pte_amount;           // 1 for PUD
    unsigned long entry_size = page_amount_round_up(entry_amount * 8);  // each entry take 8 bytes          
*/
    page_table_t page_table;
    page_table = (page_table_t)kmalloc(PAGE_SIZE);
    if(page_table == 0)
        return 0;
    memset(page_table, 0, PAGE_SIZE);
    return page_table;
}