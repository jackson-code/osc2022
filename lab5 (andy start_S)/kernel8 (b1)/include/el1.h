#ifndef __EL1_H__
#define __EL1_H__


#include "cpio.h"
#include "uart.h"
#include "my_string.h"
#include "mm.h"

//#define NEW_ADDR	((char *)0x20000)// user program addr


void el1_switch_to_el0(char *img_name);
void el1_switch_to_el0_lab5(char *img_name);


#endif
