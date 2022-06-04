#ifndef __LIST_H
#define __LIST_H

typedef struct list_head {
  struct list_head *next;
  struct list_head *prev;
} list_head;

void list_init(list_head *list);

void list_push(list_head *old, list_head *new);

void list_pop(list_head *chunk);

int list_empty(list_head *l) ;


#endif  /*_LIST_H */
