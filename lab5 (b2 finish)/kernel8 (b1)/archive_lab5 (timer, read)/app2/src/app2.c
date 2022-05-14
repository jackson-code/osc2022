#include "sys_call.h"

void delay(unsigned long num)
{
    while (num--) {}
}

//#define BUF_MAX 500

void main(void) {
    char buf[1];

    while (1)
    {
        uartwrite("read\n", 6);
        unsigned long len = uartread(buf, 1);
        uartwrite("write\n", 7);
        //uartwrite("uartread test\n", 15);
        uartwrite(buf, len);
    }
    


}


