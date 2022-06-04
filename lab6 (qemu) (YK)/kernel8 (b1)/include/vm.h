#ifndef _VM_H_
#define _VM_H_

#include "task.h"

#define PTE_V       (1L << 0)   // valid

#define PMD_GRANULE (0x200000)  // 2MB
#define PTE_GRANULE (0x1000)    // 4KB

#define PTE_SHIFT   (12)
//#define PA2PTE(pa)  (pa << PTE_SHIFT)
#define PA2PTE(pa)          ((pa    >> PTE_SHIFT) << PTE_SHIFT)
#define PTE2PA(entry)       ((entry >> PTE_SHIFT) << PTE_SHIFT)


#define IDX_SHIFT           (9)     // PAGE TABLE index width         
#define IDX_MASK            (0x1ff) // PAGE TABLE index width         
#define BYTE_ADDR           (12)    // phys byte addr take 12 bits
// Page Table index
#define PT_IDX(level, va)   (((va >> BYTE_ADDR) >> (IDX_SHIFT * level)) & IDX_MASK)


unsigned long page_amount_round_up(unsigned long size);
page_table_t page_table_create();

void update_pgd(unsigned long pgd);

#endif