#ifndef _TRAP_FRAME_H_
#define _TRAP_FRAME_H_

struct trapframe {
    unsigned long x[31]; // general register from x0 ~ x30
    unsigned long sp_el0;
    unsigned long elr_el1;
    unsigned long spsr_el1;
};

#endif