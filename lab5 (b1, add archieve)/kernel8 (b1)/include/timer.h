#ifndef __TIMER_H__
#define __TIMER_H__

#include "uart.h"
#include "el1.h"
#include "convert.h"
#include "allocator.h"

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

void timeout_event_init();
//void set_timeout(char *args);
void set_timeout(char *msg, char *arg2);
//void set_timeout();
void timer_callback(char *msg); 
//void add_timer(void (*callback)(char *), char *args, unsigned int duration);
void add_timer();
void el1_timer_irq();
void el0_timer_irq();
unsigned long get_current_time();
void set_expired_time(unsigned int duration);

void timer_time_after_booting();
#endif
