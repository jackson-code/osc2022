#ifndef __SYS_H__
#define __SYS_H__

//#include "trapframe.h"
//#include "thread.h"

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


#ifndef __ASSEMBLY__

//#include "typedef.h"


/* Function in thread.S */
//extern Task *thread_get_current();


/* Function in sys_call.S */
extern unsigned long getpid();
extern unsigned long uartread(char buf[], unsigned long size);
extern unsigned long uartwrite(const char buf[], unsigned long size);
extern int exec(const char *name, char *const argv[]);
extern int fork();
extern void exit(int status);
extern int mailbox_call(unsigned char ch, unsigned int *mailbox);
extern void kill(int pid);

extern unsigned long uart_write_int(unsigned int num);
extern unsigned long uart_write_hex(unsigned long hex);


/* Function in sys_call.c*/
unsigned int uart_printf(char* fmt,...);

#endif
#endif
