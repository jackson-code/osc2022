#include "uart.h"
#include "timer.h"

#define IRQ_PENDING_1 	 		((volatile unsigned int*)(MMIO_BASE + 0x0000b204))


