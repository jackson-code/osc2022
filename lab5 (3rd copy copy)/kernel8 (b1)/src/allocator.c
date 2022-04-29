#include "convert.h"

extern char __heap_base[];
extern char __heap_top[];
static unsigned long freep = (unsigned long)__heap_base;

// return '\0' if no space in heap
char *simple_alloc(unsigned long nbytes)
{

/*
	if ((*freep + nbytes) < *__heap_top)
	{
		char *p = freep;
		*freep += nbytes;
		return p;
	}
	else {
		return '\0';
	}
	*/
	
	unsigned long used_adder = freep + nbytes;
	if (used_adder < (unsigned long)__heap_top)
	{
		char *p = (char *)freep;
		freep += nbytes;
		return p;
	}
	else {
		return '\0';
	}
	
	/*
	unsigned long free_addr = ahtoi(freep, 4);
	char tempp[4];
	itoa(free_addr + nbytes, tempp);
	if ((tempp) < __heap_top)
	{
		char *p = freep;
		*freep += nbytes;
		return p;
	}
	else {
		return '\0';
	}
	*/
}
