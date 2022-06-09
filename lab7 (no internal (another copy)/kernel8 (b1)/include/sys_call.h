#ifndef __SYS_H__
#define __SYS_H__

#include "trapframe.h"
#include "thread.h"
#include "uart.h"
//#include "task.h"

#define SYS_GET_PID    		0
#define SYS_UART_READ       1
#define SYS_UART_WRITE      2
#define SYS_EXEC            3
#define SYS_FORK            4
#define SYS_EXIT            5
#define SYS_MBOX_CALL       6
#define SYS_KILL        	7

#define SYS_UART_WRITE_INT  8
#define SYS_UART_WRITE_HEX  9

#define SYS_OPEN            11
#define SYS_CLOSE           12
#define SYS_WRITE           13
#define SYS_READ            14
#define SYS_MKDIR           15
#define SYS_MOUNT           16
#define SYS_CHDIR           17


void sys_call_router(unsigned long sys_call_num, struct trapframe* trapframe);

/* Function in thread.S */
extern Task *thread_get_current();

/* Function in sys_call.S */
/*
extern unsigned long getpid();
extern unsigned long uartread(char buf[], unsigned long size);
extern unsigned long uartwrite(const char buf[], unsigned long size);
extern int exec(void(*func)());
extern int fork();
extern void exit(int status);
*/

// function in thread.S
extern Task *thread_get_current();
extern void thread_set_current();
extern void thread_switch();


//#endif
#endif