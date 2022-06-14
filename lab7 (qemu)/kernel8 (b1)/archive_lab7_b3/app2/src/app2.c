#include "sys_call.h"

void delay(unsigned long num)
{
    while (num--) {}
}

//#define BUF_MAX 500

void main(void) {
/*
    char buf[500];
    unsigned long len = uartread(buf, 500);
    uartwrite("uartread test\n", 15);
    uartwrite(buf, len);
    uartwrite("len : ", 7);
    uart_write_int(len);
    uartwrite("\n", 2);
    */

    uartwrite("----- app2 begin-----\n", 23);
    int fd = open("./alphabet", O_CREAT);       // create the file's vnode & file_handle
    uartwrite("fd = ", 6);
    uart_write_int(fd);
    uartwrite("\n", 1);

    char write_buf[] = "abcde";
    write(fd, write_buf, 3);
    close(fd);
    uartwrite("write_buf: ", 12);
    uartwrite(write_buf, 6);
    uartwrite("\n", 1);

    char read_buf[50];
    fd = open("./alphabet", O_CREAT);           // will find the file's vnode, and alloc for file_handle
    read(fd, read_buf, 2);
    chdir("./alphabet");                        // fail, because it's regular file vnode
    
    mkdir("./say");
    chdir("./say");
    int fd2 = open("./hello", O_CREAT);         // will in "/user/say/hello"(/user mkdir in kernel before)

    uartwrite("fd = ", 6);
    uart_write_int(fd);
    uartwrite("\tfd2 = ", 8);
    uart_write_int(fd2);
    uartwrite("\n", 1);

    close(fd);
    close(fd2);

    uartwrite("read_buf: ", 11);
    uartwrite(read_buf, 6);
    uartwrite("\n", 1);

    uartwrite("----- app2 end-----\n", 21);
}


