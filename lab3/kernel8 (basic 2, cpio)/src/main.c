#include "uart.h"
#include "shell.h"
#include "el1.h"

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

void main()
{
    // set uart
    uart_init();
    //uart_flush();    
    //uart_printf("uart init");
    
    //int c;
    //uart_printf("c = %d", c);

	int MAXCMD = 20;
    char cmd[MAXCMD]; 
    // echo everything back
    while(1) {  
        shell_get_command(cmd, MAXCMD);
        shell_execute(cmd);
    }
}
