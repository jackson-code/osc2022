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

	// test basic 4
	vfs_mkdir("/initramfs");
	vfs_mount("/initramfs", "initramfs");



	// file_t *fd = (file_t *)kmalloc(sizeof(file_t));
	// vfs_open("/ab", O_CREAT, &fd);
	// vfs_open("/ab", O_CREAT, &fd);
	// vfs_close(fd);

	// char *buf_write = "apple";
	// char buf_read[4096]; 

	// fd = (file_t *)kmalloc(sizeof(file_t));
	// vfs_open("/ab", O_CREAT, &fd);
	// vfs_open("/ab", O_CREAT, &fd);

	// vfs_write(fd, buf_write, 123);
	// vfs_close(fd);

	// fd = (file_t *)kmalloc(sizeof(file_t));
	// vfs_open("/ab", O_CREAT, &fd);
	// vfs_read(fd, buf_read, 3);
	// vfs_read(fd, buf_read+3, 3);


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
