#include "uart.h"
#include "uart_bootloader.h"


void main()
{
    // set uart
    uart_init();
    uart_flush();    
    
    while(1) {  
        char c = uart_getc();
		if (c) {
			uart_printf("====================");
			uart_printf("     Bootloader     ");
			uart_printf("====================");
		}
        loadimg();
    }
}
