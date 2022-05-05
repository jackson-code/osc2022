#include "sys_call.h"

void delay(unsigned long num)
{
    while (num--) {}
}

void main(void) {
    //const char buf[7] = "hello\n"; 
    uartwrite("hello\n", 7);
    //uart_printf("app2\n");
    //char buffer[500];
    //uart_printf("read test: \n");
    //int len=uart_read(buffer,500);
    //uart_printf("%s,len: %d\n",buffer,len);

    long long cur_sp;
    asm volatile("mov %0, sp" : "=r"(cur_sp));   
    uartwrite("Fork Test, pid = ", 18);
    uart_write_int(getpid());
    uartwrite(", sp: ", 7);
    uart_write_hex(cur_sp);
    uartwrite("\n", 2);

    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));       
        uartwrite("first child pid: ", 18);
        uart_write_int(getpid());
        uartwrite(", cnt: ", 8);
        uart_write_int(cnt);
        uartwrite(", ptr: ", 8);
        uart_write_hex((unsigned long)&cnt);
        uartwrite(", sp: ", 7);
        uart_write_hex(cur_sp);
        uartwrite("\n", 2);

        ++cnt;
/*
        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            uartwrite("first child pid: ", 18);
            uart_write_int(getpid());
            uartwrite(", cnt: ", 8);
            uart_write_int(cnt);
            uartwrite(", ptr: ", 8);
            uart_write_hex((unsigned long)&cnt);
            uartwrite(", sp: ", 7);
            uart_write_hex(cur_sp);
            uartwrite("\n", 2);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                uartwrite("second child pid: ", 19);
                uart_write_int(getpid());
                uartwrite(", cnt: ", 8);
                uart_write_int(cnt);
                uartwrite(", ptr: ", 8);
                uart_write_hex((unsigned long)&cnt);
                uartwrite(", sp: ", 7);
                uart_write_hex(cur_sp);
                uartwrite("\n", 2);

                delay(1000000);
                ++cnt;
            }
        }*/
        exit();
    }
    else {
        //printf("parent here, pid %d, child %d\n", get_pid(), ret);

        uartwrite("parent here, pid ", 18);
        uart_write_int(getpid());
        uartwrite(", child ", 9);
        uart_write_int(ret);
        uartwrite("\n", 2);
    }
}
