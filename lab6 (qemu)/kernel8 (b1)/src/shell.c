#include "shell.h"
#include "uart.h"
#include "mailbox.h"
#include "my_string.h"
#include "command.h"
#include "cpio.h"
#include "allocator.h"
#include "dtb_parser.h"
#include "el1.h"
#include "timer.h"
#include "exception.h"
#include "mm.h"
#include "thread.h"

void shell_get_command(char *cmd, int lim);
void shell_execute(char *cmd);

void shell_get_command(char *cmd, int lim)
{
	int c;
	while (--lim > 0 && (c = uart_getc()) != '\n') {
		uart_send(c);
		*cmd++ = c;
	}
	*cmd = '\0';
	
	//uart_async_puts(cmd);
	
	// print \n\r
	uart_puts("\n\r");
}


void shell_async_get_command(char *cmd, int lim)
{
	int c;
	while (--lim > 0 && (c = uart_async_getc()) != '\n') {
		*cmd++ = c;
		uart_async_putc(c);
	}
	*cmd = '\0';
	
	// newline
	uart_async_putc('\n');
	uart_async_putc('\r');
}


void shell_execute(char *cmd)
{
	if (*cmd == '\0')
		return;
	
	// get args
	int token_num = str_token_count(cmd);
	char *tokens[token_num];
	for (int j = 0; j < token_num; j++) {
		tokens[j] = simple_alloc(10);
		str_token(cmd, j, tokens[j]);
	}
	
	// debug
	/*
	int i = 0;
	while (i < token_num) {
		uart_puts(tokens[i++]);
		uart_puts("\n");
	}
	*/
		
	char *keyword = tokens[0];	
	
	if (!str_cmp(keyword, "help")) {
		uart_puts("help:\t print this help menu\n");
		uart_puts("hello:\t print Hello World!\n");
		uart_puts("reboot:\t reboot the device\n");
		uart_puts("boardinfo:\t print the raspi revision & memory base addr and size\n");
		uart_puts("ls:\t list all file (parse the .cpio)\n");
		uart_puts("cat:\t print file content (parse the .cpio)\n");
		uart_puts("alloc:\t test the function simple_alloc()\n");
		uart_puts("dt_parse:\t test the function dt_parse()\n");
		uart_puts("dt_info:\t test the function dt_info()\n");
		uart_puts("lab3_b12:\t el1_switch_to_el0(), switch EL1 to EL0, then laod user program, print current time each 2 seconds\n");
		//uart_puts("");
	}
	else if (!str_cmp(keyword, "hello")) {
	    uart_puts("I'm kernel8\n");
	}
	else if (!str_cmp(keyword, "reboot")) {
	    uart_puts("rebooting...\n");
	    reset(150);
	}
	else if (!str_cmp(keyword, "boardinfo")) {    
		// print raspi revision
		mailbox_get_board_revision();
		// print memory base addr and size
		mailbox_get_arm_memory();
	}
	else if (!str_cmp(keyword, "ls")) {
		cpio_list();	
	}
	else if (!str_cmp(keyword, "cat")) {
		uart_puts("Input file name:");
		int max_file_name = 100;
		char s[max_file_name];
		uart_get_string(s, max_file_name);
		uart_puts(s);
		cpio_cat(s);	
	}
	else if (!str_cmp(keyword, "alloc")) {
	    char *s = simple_alloc(10);
	    if (*s) {
    		s = "abcdefefg\0";
    		uart_puts(s);
	    }
    	else {
			uart_puts("no space in heap\n");
		}
	}
	else if (!str_cmp(keyword, "dt_parse")) {
		dt_parse();	
	}
	else if (!str_cmp(keyword, "dt_info")) {
		dt_info();	
	}
	else if (!str_cmp(keyword, "lab3")) {
		core_timer_enable();			// defined in timer.S
		timer_set_expired_time_by_sec(2);			// 2 seconds
		el1_switch_to_el0("user.img");	
	}
	else if (!str_cmp(keyword, "lab3_a1")) {
	 	if (token_num == 3) {
	 		set_timeout(tokens[1], tokens[2]);	
	 	}
		else {
			uart_puts("ERROR: # args not right\n");
		}	
	}
	else if (!str_cmp(keyword, "buddy")) {
		print_buddy_info();
	}
	else if (!str_cmp(keyword, "slab")) {
		print_slab();
	}
	else if (!str_cmp(keyword, "demo_alloc_page")) {
		demo_alloc_page();
	}
	else if (!str_cmp(keyword, "demo_free_page")) {
		demo_free_page();
	}
	else if (!str_cmp(keyword, "demo_kmalloc")) {
		demo_kmalloc();
	}
	else if (!str_cmp(keyword, "demo_kfree")) {
		demo_kfree();
	}
	else if (!str_cmp(keyword, "b1")) {
		threadTest1();
	}
	else if (!str_cmp(keyword, "b2")) {
		char *argv[] = {"no", "argv"};
		el1_exec("app2.img", argv);	
	}
	else if (!str_cmp(keyword, "ex")) {
		char *argv[] = {"no", "argv"};
		el1_exec("app1.img", argv);	
	}
	else if (!str_cmp(keyword, "t")) {
		core_timer_enable();
		core_timer_access_by_el0();				// video play, no ec = 0x18 error
		timer_set_expired_time_by_shift(5);
		char *argv[] = {"no", "argv"};
		el1_exec("app2.img", argv);	
	}
	else if (!str_cmp(keyword, "b3")) {
		core_timer_enable();
		core_timer_access_by_el0();				// video play, no ec = 0x18 error
		timer_set_expired_time_by_shift(5);

		char *argv[] = {"no", "argv"};
		el1_exec("syscall.img", argv);	
	}
	else {
		uart_puts("ERROR: unsupport shell command\n");
	}
}
