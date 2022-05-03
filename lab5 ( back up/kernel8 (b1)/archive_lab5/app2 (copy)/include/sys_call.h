#ifndef __SYS_H__
#define __SYS_H__

//#include "trapframe.h"
//#include "thread.h"

#define SYS_GET_TASK_ID     0
#define SYS_UART_READ       1
#define SYS_UART_WRITE      2
#define SYS_EXEC            3
#define SYS_FORK            4
#define SYS_EXIT            5

#ifndef __ASSEMBLY__

//#include "typedef.h"


/* Function in thread.S */
//extern Task *thread_get_current();


/* Function in sys_call.S */
extern unsigned long getpid();
extern unsigned long uartread(char buf[], unsigned long size);
extern unsigned long uartwrite(const char buf[], unsigned long size);
extern int exec(void(*func)());
extern int fork();
extern void exit(int status);

#endif
#endif