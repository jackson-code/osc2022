#ifndef __LIST_H
#define __LIST_H

typedef struct list_head {
  struct list_head *next;
  struct list_head *prev;
} list_head;

#define offsetof(TYPE, MEMBER) ((unsigned long)&((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
               (type *)( (char *)__mptr - offsetof(type,member) ); })

#define list_entry(ptr, type, member) container_of(ptr, type, member)


void list_init(list_head *list);

void list_push(list_head *old, list_head *new);

void list_pop(list_head *chunk);

int list_empty(list_head *l) ;


#endif  /*_LIST_H */
