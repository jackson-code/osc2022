#ifndef _MMIO_H_
#define _MMIO_H_

#include "vm.h"

// ch 1.2.3 (physical address)
#define MMIO_PHYS_START     (0x3F000000)
#define MMIO_PHYS_END       (0x40000000)
#define MMIO_BASE           (KERNEL_VIRT_BASE | MMIO_PHYS_START)

#endif