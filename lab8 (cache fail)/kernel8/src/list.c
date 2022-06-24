#include "list.h"

void list_init(list_head *l) {
  l->next = l;
  l->prev = l;
}

void list_push(list_head *old,list_head *new) {
  new->prev = old;
  new->next = old->next;
  old->next->prev = new;
  old->next = new;
}

void list_pop(list_head *chunk) {
  chunk->next->prev = chunk->prev;
  chunk->prev->next = chunk->next;
}

int list_empty(list_head *l) { 
	return l->next == l; 
}



