#ifndef _IRQ_H
#define _IRQ_H

#include "uart.h"
#include "timer.h"

#define IRQ_PENDING_1 	 		((volatile unsigned int*)(MMIO_BASE + 0x0000b204))

void enable_interrupt();
void disable_interrupt();


#endif
