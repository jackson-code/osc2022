#include "uart_bootloader.h"
#include "uart.h"

void load_new_kernel();

extern char __start[];		
extern char __end[];

void bootloader_self_relocation();

void loadimg(){
	bootloader_self_relocation();
}

void bootloader_self_relocation(){
	char *cur_addr = __start, *end_addr = __end, *target_addr = TARGET_ADDR;
	
	uart_printf("Copy bootloader(old kernel) to 0x60000(TARGET_ADDR)");
	while (cur_addr <= end_addr) {
		*target_addr++ = *cur_addr++;
	}
	uart_printf("finish");

    // calculate the new func addr
    void (*call_load_new_kernel)(void) = (void (*)(void))((unsigned long int)load_new_kernel - (unsigned long int)__start + TARGET_ADDR);
    call_load_new_kernel();   
}

void load_new_kernel(){   
    // get new img size through host2pi.py
    uart_printf("Please kill the screen, press ctrl+a, k, y");
    uart_printf("Please execute host2pi.py to send new kernel size and content from host, and pi will receive new kernel through uart");
    int size = 0;
    while (size == 0) {
    	size = uart_get_int();
    }

    char *current_addr = KERNEL_ADDR;
	char c;
    for(int i = 0; i < size; ++i){
        c = uart_getc_raw();
        *current_addr++ = c;
    }
    
    // execute start.S
    void (*new_kernel_start)(void) = (void (*)(void))KERNEL_ADDR;
    new_kernel_start();
}
