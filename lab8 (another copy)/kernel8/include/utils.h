#ifndef _UTILS_H
#define _UTILS_H

#include "typedef.h"

#define NULL                    ((void *)0) 

void *memset(void *s, int c, unsigned long n);
void memcpy(void *old, void *new, unsigned long size);
// void memcpy(char *old, char *new, unsigned long size);

int strcmp(const char *a, const char *b);
int strlen(const char* s);
unsigned long hextoint(char* addr, const int size);
void swap(void **a, void **b) ;

#endif /*_UTILS_H */
