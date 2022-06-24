#ifndef __EL1_H__
#define __EL1_H__

//#define NEW_ADDR	((char *)0x20000)// user program addr

void el1_switch_to_el0(char *img_name);
void el1_exec(char * img_name, char *argv[]);

void el1_exec_lab7_a1(char *img_name);
void el1_exec_lab7_test(char *pathname);

#endif
