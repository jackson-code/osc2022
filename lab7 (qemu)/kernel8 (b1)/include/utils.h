#ifndef _UTILS_H
#define _UTILS_H

#define NULL                    ((void *)0) 

void *memset(void *s, int c, unsigned long n);
int strcmp(const char *a, const char *b);
int strlen(const char* s);
unsigned long hextoint(char* addr, const int size);
void swap(void **a, void **b) ;
void copy(char *old, char *new, unsigned long size);

#endif /*_UTILS_H */
