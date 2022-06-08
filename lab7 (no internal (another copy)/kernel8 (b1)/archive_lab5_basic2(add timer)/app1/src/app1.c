#include "sys_call.h"

void delay(unsigned long num)
{
    while (num--) {}
}

//#define BUF_MAX 500

void main(void) {
    uartwrite("app1 exec app2.img\n", 20);

    char *const argv[] = {"no", "arg"};
    if (exec("app2.img", argv) == -1) {
        uartwrite("ERROR in app1.c, fail to exec app2.img \n", 41);
    }
}


