#include "sys_call.h"

void delay(unsigned long num)
{
    while (num--) {}
}

//#define BUF_MAX 500

void main(void) {
    char buf[500];
    unsigned long len = uartread(buf, 500);
    uartwrite("uartread test\n", 15);
    uartwrite(buf, len);
    uartwrite("len : ", 7);
    uart_write_int(len);
    uartwrite("\n", 2);

    //uartwrite("mbox test\n", 15);
    //mailbox_get_arm_memory();

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

            //kill(1);
        }
        //kill(0);
        exit(0);
    }
    else {
        uartwrite("parent here, pid ", 18);
        uart_write_int(getpid());
        uartwrite(", child ", 9);
        uart_write_int(ret);
        uartwrite("\n", 2);
    }
}


