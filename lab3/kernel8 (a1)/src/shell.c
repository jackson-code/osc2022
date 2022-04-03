#include "shell.h"

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
	uart_puts("\0");
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
	int i = 0;
	while (i < token_num) {
		uart_puts(tokens[i++]);
	}
		
	char *keyword = tokens[0];	
	
	if (!str_cmp(keyword, "help")) {
		uart_puts("help:\t print this help menu");
		uart_puts("hello:\t print Hello World!");
		uart_puts("reboot:\t reboot the device");
		uart_puts("boardinfo:\t print the raspi revision & memory base addr and size");
		uart_puts("ls:\t list all file (parse the .cpio)");
		uart_puts("cat:\t print file content (parse the .cpio)");
		uart_puts("alloc:\t test the function simple_alloc()");
		uart_puts("dt_parse:\t test the function dt_parse()");
		uart_puts("dt_info:\t test the function dt_info()");
		uart_puts("b12:\t el_user_start(), switch EL1 to EL0, then laod user program, print current time each 2 seconds");
		//uart_puts("");
	}
	else if (!str_cmp(keyword, "hello")) {
	    uart_puts("I'm kernel8");
	}
	else if (!str_cmp(keyword, "reboot")) {
	    uart_puts("rebooting...");
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
			uart_puts("no space in heap");
		}
	}
	else if (!str_cmp(keyword, "dt_parse")) {
		dt_parse();	
	}
	else if (!str_cmp(keyword, "dt_info")) {
		dt_info();	
	}
	else if (!str_cmp(keyword, "b12")) {
		core_timer_enable();			// defined in timer.S
		set_expired_time(2);			// 2 seconds
		el_user_start();	
	}
	else if (!str_cmp(keyword, "a1")) {
	 	if (token_num == 3) {
	 		set_timeout();	
	 	}
		else {
			uart_puts("ERROR: # args not right");
		}	
	}
	else {
		uart_puts("ERROR: unsupport shell command");
	}
}
