#include "sys_call.h"

void delay(unsigned long num)
{
    while (num--) {}
}

//#define BUF_MAX 500

void main(void) {
    uartwrite("----- app2 begin-----\n", 23);
    
    char write_buf[] = "abcde\n";
    write(1, write_buf, 6);

    uartwrite("enter 5 char: ", 15);
    
    char read_buf[50];
    int len = read(0, read_buf, 5);

    uartwrite(read_buf, len);
    uartwrite("\n", 1);

    uartwrite("----- app2 end-----\n", 21);
}


