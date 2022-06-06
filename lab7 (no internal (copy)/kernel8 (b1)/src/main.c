#include "uart.h"
#include "shell.h"
#include "el1.h"
#include "exception.h"
#include "cpio.h"
#include "mm.h"
#include "vfs.h"

/*
unsigned long __stack_chk_guard;
void __stack_chk_guard_setup(void)
{
     __stack_chk_guard = 0xBAAAAAAD;//provide some magic numbers
}

void __stack_chk_fail(void)                         
{                              
}
*/

extern int uart_interrupt;

void main()
{
	// set uart
	uart_init();
	//uart_flush();    
	
 	init_memory_system();
  	cpio_init();

	rootfs_init();

	// test basic
	file_t **fd = 0;
	vfs_open("/ab", O_CREAT, fd);
	vfs_open("/ab", O_CREAT, fd);

	char *buf = "apple";
	vfs_write(*fd, buf, 123);

	vfs_open("/ab", O_CREAT, fd);
	vfs_open("/ab", O_CREAT, fd);

	int MAXCMD = 20;
  	char cmd[MAXCMD]; 
    
	if (uart_interrupt)
	{
		enable_interrupt();
		while(1) {
		shell_async_get_command(cmd, MAXCMD);
		shell_execute(cmd);
		}	
	}
	else 
	{
		while(1) {  
		shell_get_command(cmd, MAXCMD);
		shell_execute(cmd);
		}
	}
}
