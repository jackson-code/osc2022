#ifndef	_MM_H_
#define	_MM_H_

#include "list.h"



#define BUDDY_MAX 				16    	// 4kB ~ 128MB (128MB = 4kb * 2^15 / 1024)
#define STARTUP_MAX 			16  	// max reserve slot
#define PAGE_SIZE 				4096	// 4KB
#define PAGE_SIZE_CTZ 			12		// CTZ means count trailing zero, page size = 4kb = 4096b = 1 0000 0000 0000 
#define INIT_PAGE 				32

#define pad(x, y) ((((x) + (y)-1) / (y)) * (y))
/*
convert byte addr to order of 2 unit in page addr, then use the page addr as the idx in free_list
order of 2 = trailing zero in page addr
if ptr = 0, the result is undefined!
*/
#define get_order(ptr) (__builtin_ctzl((unsigned long)ptr) - PAGE_SIZE_CTZ)
#define set_buddy_ord(bd, ord) (bd = ord | (bd & 0xe0))
#define set_buddy_flag(bd, flag) (bd = (flag << 5) | (bd & 0x1f))
#define get_buddy_ord(bd) (bd & 0x1f)
#define get_buddy_flag(bd) ((bd & 0xe0) >> 5)
#define ptr_to_pagenum(ptr) (((unsigned long)(ptr)) >> PAGE_SIZE_CTZ)
#define pagenum_to_ptr(pn) ((void *)(((unsigned long)pn) << PAGE_SIZE_CTZ))
/*
 find page num of buddy(beside page)
 e.g. if ord = 0(4KB), then [ pg, ((pg) ^ (1 << ord)) ] = 
 [0, 1], [2, 3], [4, 5], [6, 7], ...
  e.g. if ord = 1(8KB), then [ pg, ((pg) ^ (1 << ord)) ] = 
 [0, 2], [4, 6], ...
*/
#define buddy_pagenum(pg, ord) ((pg) ^ (1 << ord))


enum{
	BUDDY_FREE,  			//0
	BUDDY_USE,				//1
	SLAB_USE,				//2
	RESRVE_USE				//3
};


typedef struct buddy_system {
  list_head free_list[BUDDY_MAX];
} buddy_system;

typedef struct startup_allocator {
  unsigned long addr[STARTUP_MAX];
  unsigned long size[STARTUP_MAX];
} startup_allocator;

typedef struct cache_list {
  struct cache_list *next;
} cache_list;

// 每個page_descriptor都各自指向一個page，且page_descriptor集中放在一個或多個page，而不是各自放在指向的page中
// 而被用來放page_descriptor的page，也會有一個page_descriptor去指向他
typedef struct page_descriptor {
  void *page;						// point to the first addr of a page
  struct page_descriptor *next_pd;
  cache_list *free_list;
  unsigned int free_count;
} page_descriptor;

// 
typedef struct slab_cache {
  struct slab_cache *next_slab;
  page_descriptor *head_pd;
  page_descriptor *cache_pd;		// page_descriptor
  void *page_slice_pos;
  unsigned int size;				// size in byte, maybe 16 byte aligned?
  unsigned int free_count;
  unsigned int page_remain;			// remain bytes in a page
} slab_cache;

buddy_system 		buddy;
startup_allocator 	startup;
char *				buddy_stat;
slab_cache *		slab_st;		// slab start, the first slab
slab_cache *		sc_slab_tok;	// 永遠指向sc_slab，負責管理slab_cache type
slab_cache *		pd_slab_tok;	// 當要分配pd時，會放在pd_slab_tok->pd_head所指向的page中

void *kmalloc(unsigned long size);
void *alloc_page(unsigned int size);
void *alloc_slab(void *slab_tok);

void kfree(void *ptr);
void free_page(void *ptr);
void free_reserve(void *ptr);
void free_unknow_slab(void *ptr);

void init_memory_system();

void print_buddy_info();
void print_buddy_stat();
void print_slab();

void test_buddy();
void test_slab();

// for demo
void demo_alloc_page();
void demo_free_page();
void demo_kmalloc();
void demo_kfree();


#endif  /*_BUDDY_H */
