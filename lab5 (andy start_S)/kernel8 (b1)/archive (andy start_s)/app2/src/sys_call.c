#include "sys_call.h"
#include "printf.h"

unsigned long uartwrite(const char *buf, unsigned long size)
{
    /*
    asm volatile("mov	x0, %0		\n"::"r"(buf));
    asm volatile("mov	x1, %0		\n"::"r"(size));
    asm volatile("mov   x8, %0   	\n"::"m"(2));
    asm volatile("svc   #0      	\n");
    asm volatile("ret		        \n");
    */
   /*
    asm volatile("mov	x0, %0		\n"
                "mov	x1, %1		\n"
                "mov    x8, %2   	\n"
                "svc   #0       	\n"
                "ret		          "
                ::"r"(buf), "r"(size), "r"(2));
                */

    asm volatile("mov	x0, %0		\n"
                "mov	x1, %1		\n"
                "mov	x2, %2		\n"
                "mov    x8, %3   	\n"
                "svc   #0       	\n"
                "ret		          "
                ::"r"(buf), "r"(size), "r"(buf),"r"(2));


/*
    int res = 0;
    asm volatile("add	%0, x0		\n":"=m"(res));
    return res;
*/
    return 0;
}

unsigned int uart_printf(char* fmt,...){
	char dst[100];
    //__builtin_va_start(args, fmt): "..." is pointed by args
    //__builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args,fmt);
    unsigned int ret=vsprintf(dst,fmt,args);
    uartwrite(dst,ret);
    return ret;
}