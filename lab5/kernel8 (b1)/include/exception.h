#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "uart.h"
#include "timer.h"
#include "sys_call.h"
#include "trapframe.h"

#define IRQ_PENDING_1 	 		((volatile unsigned int*)(MMIO_BASE + 0x0000b204))

void enable_interrupt();
void disable_interrupt();


#endif // _EXCEPTION_H_
