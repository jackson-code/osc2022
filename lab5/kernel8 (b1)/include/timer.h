#ifndef __TIMER_H__
#define __TIMER_H__

#include "uart.h"
#include "el1.h"
#include "convert.h"
#include "allocator.h"
#include "trapframe.h"

// #define TIME_FREQ 0x03b9aca0

extern void core_timer_enable (void);
extern void core_timer_disable (void);
extern unsigned long get_current_timer_cnt();
extern unsigned long get_timer_freq();

typedef struct timeout_event {
  unsigned int register_time;
  unsigned int duration;
  void (*callback)(char *);
  char args[20];
  struct timeout_event *prev, *next;
} timeout_event;

timeout_event *timeout_queue_head, *timeout_queue_tail;

void core_timer_access_by_el0();

void timeout_event_init();
//void set_timeout(char *args);
void set_timeout(char *msg, char *arg2);
//void set_timeout();
void timer_callback(char *msg); 
//void add_timer(void (*callback)(char *), char *args, unsigned int duration);
void add_timer();
//void timer_irq_el1();
void timer_irq_el0(struct trapframe *trapframe);
void timer_irq_el1(struct trapframe *trapframe);
//void timer_irq_el1(struct trapframe *trapframe);
unsigned long get_current_time();
// set time irq period
void timer_set_expired_time_by_sec(unsigned int duration);
void timer_set_expired_time_by_shift(unsigned int shift);

void timer_time_after_booting();
#endif
