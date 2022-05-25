#ifndef _VM_H_
#define _VM_H_

#define KERNEL_VIRT_BASE (0xffff000000000000)

/*
    Translation Control Register (TCR)¶
*/
// space size (addressable range)
#define T0SZ_48_BIT ((64 - 48) << 0)        // set addressable range 48 bits in translation table 0
#define T1SZ_48_BIT ((64 - 48) << 16)       // set addressable range 48 bits in translation table 1
#define TCR_CONFIG_REGION_48bit ( T0SZ_48_BIT | T1SZ_48_BIT)
// translation granule
#define TG_4KB              (0b00)
#define TG_UNDEFINE_KB      (0b10)
#define TG0                 (TG_4KB << 14)
#define TG1                 (TG_UNDEFINE_KB << 30)
#define TCR_CONFIG_4KB      (TG0 | TG1)
// TCR_EL1 config
#define TCR_CONFIG_DEFAULT  (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

/*
    Memory Attribute Indirection Register (MAIR)
    ref: DDI0487A_a_armv8_arm.pdf
*/
#define MAIR_DEVICE_nGnRnE      (0b00000000)    // Device-nGnRnE memory (non-Gathering, non-Reordering, No Early write acknowledgement)
#define MAIR_NORMAL_noCACHE     (0b01000100)    // Normal memory, Inner and Outer Non-Cacheable
#define MAIR_IDX_DEVICE_nGnRnE  (0)
#define MAIR_IDX_NORMAL_noCACHE (1)
#define MAIR_ATTR0      (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8))
#define MAIR_ATTR1      (MAIR_NORMAL_noCACHE << (MAIR_IDX_NORMAL_noCACHE * 8))
#define MAIR_ATTR       (MAIR_ATTR0 | MAIR_ATTR1)

/*
    Page table Descriptor 
*/
// bit[1:0]
#define PD_TABLE        (0b11)
#define PD_BLOCK        (0b01)
#define PD_PAGE         (0b11)
// bit[10]
#define PD_ACCESS       (1 << 10)       // access flag
// arrtibute
#define PGD_ATTR            (PD_TABLE)
#define PUD_TABLE_ATTR      (PD_TABLE)
#define PUD_DEVICE_ATTR     (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)   
#define PMD_TABLE_ATTR      (PD_TABLE)
#define PTE_DEVICE_ATTR     (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_PAGE)            
#define PTE_NORMAL_ATTR     (PD_ACCESS | (MAIR_IDX_NORMAL_noCACHE << 2) | PD_PAGE)            


#endif