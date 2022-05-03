#include "mm.h"
#define debug

extern unsigned char __start, __end;
unsigned long mem_size = 0x40000000;  // 1 GB
unsigned long reserve_count = 0;


////////////////////////////////////////////////////////////////////////////////////////////////
//                                          reserve                                           // 
////////////////////////////////////////////////////////////////////////////////////////////////
void init_reserve_count(){
	reserve_count = 0;
}
unsigned long check_reserve_collision(unsigned long a1, unsigned int s1, unsigned long a2, unsigned int s2) {
  unsigned long e1 = (unsigned long)(a1 + s1);
  unsigned long e2 = (unsigned long)(a2 + s2);
  return ((a2 >= a1) && (a2 < e1)) || ((e2 > a1) && (e2 <= e1)) ||
         ((a1 >= a2) && (a1 < e2)) || ((e1 > a2) && (e1 <= e2));
}
int reserve_mem(unsigned long addr, unsigned long size) {
  //uart_put_int(reserve_count);
  //uart_puts("\n");

  // 從0開始算的話，4096(page size) = 0xfff
  if ((addr & 0xfff) != 0 || (size & 0xfff) != 0) {
    uart_puts("reserve mem require page align\n");
    return -1;
  }

  if (reserve_count >= STARTUP_MAX) {
    uart_puts("no reserve slot available\n");
    return -1;
  } else {
    for (int i = 0; i < reserve_count; i++) {
      if (check_reserve_collision((unsigned long)startup.addr[i], startup.size[i], (unsigned long)addr, size)) {
        uart_puts("reserve collision\n");
        return -1;
      }
    }
    startup.addr[reserve_count] = addr;
    startup.size[reserve_count] = size;
    reserve_count++;
    return 0;
  }
}

// a3
// just simply hardcode memory information (You can either get the it from the devicetree)
void init_startup(){
	reserve_count = 0;
	//uart_puts("reserve spin table\n");
	reserve_mem(0x0, 0x1000);  														// spin table
	//uart_puts("spin table reserve finish\n");

  //reserve_mem(0x20000, 0x10000);

	reserve_mem(0x30000, 0x20000);  														// app2 (user program)
	reserve_mem(0x60000, 0x10000);  														// bootloader 

	//uart_puts("reserve kernel\n");
	reserve_mem((unsigned long)&__start, (unsigned long)(&__end - &__start)); 		// kernel
	//uart_puts("kernel reserve finish\n");
	//uart_puts("reserve buddy system\n");
	reserve_mem((unsigned long)&__end, mem_size / PAGE_SIZE);     					// buddy system
	//uart_puts("buddy system reserve finish\n");
	//uart_puts("reserve cpio\n");
  if (property_qemu)
    reserve_mem(0x8000000,0x10000000);												// cpio when using QEMU
  else
	  reserve_mem(0x20000000,0x10000000);												// cpio
	//uart_puts("cpio reserve finish\n");
	//uart_puts("reserve dtb\n");
	reserve_mem(0x31000000,0x1000000);                  							// dtb
	//uart_puts("dtb reserve finish\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////
//                                          init                                              // 
////////////////////////////////////////////////////////////////////////////////////////////////
/*
 set buddy_stat and add into free_list
*/
void _place_buddy(unsigned long ptr, int ord, int flag) {
  unsigned long idx = ptr >> PAGE_SIZE_CTZ;			// ptr is byte addr, convert unit to page, because buddy_stat unit in page
  set_buddy_ord(buddy_stat[idx], ord);				// ord stored in bit0~4 in buddy_stat
  set_buddy_flag(buddy_stat[idx], flag);			// flag stored in bit5~7 in buddy_stat
  if (flag == BUDDY_FREE) {
    push_list(&buddy.free_list[ord], (list_head *)ptr);
  }
}
void place_buddy(unsigned long st, unsigned long ed, int flag) {
  while (st != ed) {
    int st_ord = get_order(st);		
    int ed_ord = get_order(ed);
    if (st_ord >= BUDDY_MAX) {
      st_ord = BUDDY_MAX - 1;
    }
    if (ed_ord >= BUDDY_MAX) {
      ed_ord = BUDDY_MAX - 1;
    }
    if (st_ord <= ed_ord) {
      _place_buddy(st, st_ord, flag);
	  // st已加進free list且buddy stat也設好了，表示st~st+2^ord已加進buddy system，因此可以處理st+2^ord~st+2*2^ord
      st += (1 << (st_ord + PAGE_SIZE_CTZ));	// st = st+2^ord
												// assume st(or ed) = x...x10...0, 從LSB數過來第一個1，遇到(st_ord + PAGE_SIZE_CTZ)個0
      											// (1 << (st_ord + PAGE_SIZE_CTZ)) = 10...0
    } 
	else {
      ed -= (1 << (ed_ord + PAGE_SIZE_CTZ));
      _place_buddy(ed, ed_ord, flag);
    }
  }
}
void init_buddy(unsigned long stat_ptr) {
  buddy_stat = (char*)stat_ptr;
  uart_puts("buddy system start address: ");
  uart_put_hex(stat_ptr);						// stat_ptr = __end
  uart_puts("\n");

  // 1.
  // 初始化free list的每個list head，head的prev, next都指向自己
  for (int i = 0; i < BUDDY_MAX; i++) {
    list_init((void*)&(buddy.free_list[i]));
  }

  // 2.
  // buddy_stat的數量 = page數量 = mem_size / PAGE_SIZE
  // an entry of buddy_stat will store ord and flag 
  // 不知為何設成INIT_PAGE(=32=10,0000 => BUDDY_USE, ord = 0)
  for (unsigned long i = 0; i < mem_size / PAGE_SIZE; i++) {
    buddy_stat[i] = INIT_PAGE;
  }

  // 3.
  // startup 記錄了reserve memory起始地址和大小，可知startup不包含的地址區間為將來可分配出去的地址
  unsigned long mem_itr = 0;
  for (int i = 0; i < reserve_count; i++) {

  	// startup不包含的地址區間
  	// mark the free physical addr BUDDY_FREE in the buddy_stat, 
  	// and add to the free list
    place_buddy(mem_itr, startup.addr[i], BUDDY_FREE);
	
	// mark the reserve physical addr RESRVE_USE in the buddy_stat
    place_buddy(startup.addr[i], startup.addr[i] + startup.size[i], RESRVE_USE);
  
    mem_itr = startup.addr[i] + startup.size[i];
  }
  place_buddy(mem_itr, (unsigned long)mem_size, BUDDY_FREE);
}

void init_slab() {
  uart_puts("(mm.c, init_slab)\n");
  // slab_cache for slab_cache type
  slab_cache *sc_slab = (slab_cache *)alloc_page(PAGE_SIZE);
  // slab_cache for page_descriptor type
  slab_cache *pd_slab = sc_slab + 1;
  // page_descriptor for slab_cache type
  page_descriptor *sc_page = (page_descriptor *)alloc_page(PAGE_SIZE);
  // page_descriptor for page_descriptor type
  page_descriptor *pd_page = sc_page + 1;

  set_buddy_flag(buddy_stat[ptr_to_pagenum((void *)sc_slab)], SLAB_USE);//mark buddy_stat to SLAB_USE
  set_buddy_flag(buddy_stat[ptr_to_pagenum((void *)sc_page)], SLAB_USE);//mark buddy_stat to SLAB_USE

  sc_slab->next_slab = pd_slab;
  sc_slab->free_count = 0;
  sc_slab->cache_pd = sc_page;
  sc_slab->page_remain = PAGE_SIZE - pad(sizeof(slab_cache), 16) * 2;	// 2: one for sc_slab, one for pd_slab
  sc_slab->page_slice_pos = pd_slab + 1;
  sc_slab->head_pd = sc_page;
  sc_slab->size = pad(sizeof(slab_cache), 16);

  pd_slab->next_slab = NULL;
  pd_slab->free_count = 0;
  pd_slab->cache_pd = pd_page;
  // remaining size of the page which contain pd_page(head_pd) & sc_page
  pd_slab->page_remain = PAGE_SIZE - pad(sizeof(page_descriptor), 16) * 2;	// 2: one for sc_page, one for pd_page
  pd_slab->page_slice_pos = pd_page + 1;
  pd_slab->head_pd = pd_page;
  pd_slab->size = pad(sizeof(page_descriptor), 16);

  sc_page->free_list = NULL;
  sc_page->next_pd = NULL;
  sc_page->page = (void *)sc_slab;
  sc_page->free_count = 0;

  pd_page->free_list = NULL;
  pd_page->next_pd = NULL;
  pd_page->page = (void *)sc_page;	// wrong? maybe is pd_slab 
  pd_page->free_count = 0;

  slab_st = sc_slab;
  sc_slab_tok = sc_slab;
  /* pd_slab_tok負責管理全部的page_descriptor，head_pd指向目前正在使用的page，正在使用
  指的是當需要alloc時，會alloc到此page，藉由head_pd的next_pd可以找到之前使用完(alloc到
  沒空間了)的page */
  pd_slab_tok = pd_slab;	
}
void init_memory_system(){
  uart_puts("---------- mm.c, init_memory_system() ----------\n");
  
	//reserve
  init_startup();
	//buddy
	init_buddy((unsigned long)&__end);
	//slab
	init_slab();

  uart_puts("---------- mm.c, init_memory_system() ----------\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                         kmalloc                                            // 
////////////////////////////////////////////////////////////////////////////////////////////////
/*
 1. find order which is large enough and its element existing in the free_list
 2. pop(delete) from the free_list
 3. renew the buddy_stat to BUDDY_USE
 4. release extra buddy to free_list if allocing size > size
*/
void *alloc_page(unsigned int size) {
  uart_puts("\t(alloc_page)\n");
  uart_puts("\talloc size: ");
  uart_put_int(size);
  uart_puts(" byte\n");
  size = pad(size, PAGE_SIZE);

  // page addr(!= 0) have 51 leading 0-bits at most
  unsigned int target_ord = 51 - __builtin_clzl(size);
  // target_ord + 12 = # leading 0-bits
  // (1 << (target_ord + 12)) - 1) = 1...1, # 1 = # leading 0-bits
  // search for large enough order
  if ((((1 << (target_ord + 12)) - 1) & size) != 0) {
    target_ord++;
  }
  //uart_puts("target alloc order: ");
  //uart_put_int(target_ord);
  //uart_puts("\n");

  // search memory in free list by ord
  unsigned int find_ord = target_ord;
  while (list_empty(&buddy.free_list[find_ord])) {	//if is empty
  	uart_put_int(find_ord);
  	//uart_puts(" order has no element in free_list\n");
    find_ord++;
    if (find_ord >= BUDDY_MAX) {
      uart_puts("\tWARNING: out of memory\n");					// happen when size > 128 mb or reserved memory occupy all memory
      return NULL;
    }
  }

  // get memory from buddy system, remove the memory from free list
  void *new_chunk = (void *)buddy.free_list[find_ord].next;
  pop_list((list_head *)new_chunk);

  // set buddy_stat
  unsigned int pn = ptr_to_pagenum(new_chunk);		// pagenum = addr in page addressed
  set_buddy_flag(buddy_stat[pn], BUDDY_USE);
  set_buddy_ord(buddy_stat[pn], target_ord);

  // e.g. size=4KB, but free_list[0] = free_list[1] = free_list[2] = 0, free_list[3] > 0
  // so target_ord = 0, find_ord = 3
  // while(3 > 0) { bd = idx of buddy in 16KB(右邊的buddy), release the 16KB(add into free_lsit) }
  // while(2 > 0) { bd = idx of buddy in 8KB (右邊的buddy), release the 8KB (add into free_lsit) }
  // while(1 > 0) { bd = idx of buddy in 4KB (右邊的buddy), release the 4KB (add into free_lsit) }
  // release smaller order memory 
  while (find_ord > target_ord) {
    find_ord--;
    unsigned int bd = buddy_pagenum(pn, find_ord);	// pagenum of buddy in lower(smaller ord) layer
    set_buddy_flag(buddy_stat[bd], BUDDY_FREE);		
    set_buddy_ord(buddy_stat[bd], find_ord);
    push_list(&buddy.free_list[find_ord], (list_head *)pagenum_to_ptr(bd));
    
    uart_puts("\trelease order ");	
  	uart_put_int(find_ord);
  	uart_puts("'s buddy: ");
  	uart_put_hex(bd);
  	uart_puts("\n");
  }

  uart_puts("\tfind alloc order: ");
  uart_put_int(find_ord);
  uart_puts("\n"); 
  return new_chunk;
}
void *pop_cache(cache_list **cl) {
  void *addr = (void *)*cl;
  *cl = (*cl)->next;
  return addr;
}
void *pop_slab_cache(slab_cache *sc) {
  if (sc->cache_pd->free_list == NULL) {
    page_descriptor *pd_itr = sc->head_pd;
    while (pd_itr->free_list == NULL) {
      pd_itr = pd_itr->next_pd;
    }
    sc->cache_pd = pd_itr;
  }
  page_descriptor *pd = sc->cache_pd;
  void *new_chunk = pop_cache(&(pd->free_list));
  pd->free_count--;
  sc->free_count--;
  uart_puts("pop slab cache\n");
  return new_chunk;
}
/*
 當pd_slab_tok所指向的pd所在的page沒空間時，呼叫此func
 分配新的page，並將pd放在首地址
*/
void pd_self_alloc() {
  page_descriptor *new_pd = (page_descriptor *)alloc_page(PAGE_SIZE);
  set_buddy_flag(buddy_stat[ptr_to_pagenum((void *)new_pd)], SLAB_USE);
  new_pd->page = (void *)new_pd;
  new_pd->free_count = 0;
  new_pd->free_list = NULL;
  new_pd->next_pd = pd_slab_tok->head_pd;
  pd_slab_tok->head_pd = new_pd;
  pd_slab_tok->page_remain = PAGE_SIZE - pad(sizeof(page_descriptor), 16);
  pd_slab_tok->page_slice_pos = new_pd + 1;
  uart_puts("pd self allocate\n");
}
/*
 1. create new pd or just use old pd
 2. alloc page, and the pd descript the page
*/
page_descriptor *new_pd() {
  page_descriptor *pd;
  if (pd_slab_tok->free_count > 0) {										// get old pd?
    pd = (page_descriptor *)pop_slab_cache(pd_slab_tok);
  } 
  else {																	// get new pd
    if (pd_slab_tok->page_remain < pad(sizeof(page_descriptor), 16)) {
        pd_self_alloc();		// alloc pd for new page(new page專門放pd)
    }

	// alloc pd for caller
    pd_slab_tok->page_remain -= pad(sizeof(page_descriptor), 16);
    pd = (page_descriptor *)(pd_slab_tok->page_slice_pos);
    pd_slab_tok->page_slice_pos += pad(sizeof(page_descriptor), 16);
  }
  pd->free_count = 0;
  pd->free_list = NULL;
  pd->next_pd = NULL;
  pd->page = alloc_page(PAGE_SIZE);
  set_buddy_flag(buddy_stat[ptr_to_pagenum(pd->page)], SLAB_USE);
  /*
  uart_puts("new pd at: ");
  uart_put_hex((unsigned long)pd);
  uart_puts("\n");
  */
  uart_puts("new page at: ");
  uart_put_hex((unsigned long)pd->page);
  uart_puts("\n");
  return pd;
}
void *slice_remain_slab(slab_cache *sc) {
  sc->page_remain -= sc->size;
  void *addr = sc->page_slice_pos;
  sc->page_slice_pos += sc->size;
  uart_puts("slice remain: ");
  uart_put_hex(sc->page_remain);
  uart_puts("\n");
  return addr;
}
/*
 從此slab底下的page中，分配記憶體
*/
void *alloc_slab(void *slab_tok) {
  uart_puts("\t(alloc_slab)\n");

  slab_cache *sc = (slab_cache *)slab_tok;
  if (sc->free_count > 0) {
    return pop_slab_cache(sc);
  }

  // slab_tok指向的head_page沒有剩餘的空間，因此要alloc page，
  // 表示也需要新的pd
  if (sc->page_remain < sc->size) {
    if (sc == pd_slab_tok) {					// alloc a new page & new pd, 當之後有新的pd要分配時，會藉由pd_slab_tok找到此new page，將新的pd分配到此new page
      pd_self_alloc();
    } 
	else {	
      page_descriptor *pd = new_pd();			
      pd->next_pd = sc->head_pd;
      sc->head_pd = pd;
      sc->page_slice_pos = sc->head_pd->page;	// has alloced new page in new_pd, so remain size is the whole page
      sc->page_remain = PAGE_SIZE;
    }
  }

  // alloc memory!
  return slice_remain_slab(sc);
}

/*
 觀念: 相同size的請求，會放在slab_cache->size = size 的 slab_cache->head_pd->page中
 如果找不到size相同的slab, 則
 建立新的slab_cache，並分配pd及新的page給他
*/
void *register_slab(unsigned int size) {
  // searching existing slab, 
  // if size of the existing slab is same as size, then return slab
  slab_cache *sc = slab_st;
  while (sc != NULL) {
    if (sc->size == size) {
      uart_puts("find token\n");
      return (void *)sc;
    }
    sc = sc->next_slab;
  }

  // no existing slab has the same size,
  // so alloc a new slab_cache to a addr which finded through sc_slab_tok(所有slab_cache都歸sc_slab_tok管理)
  // a new slab_cache needs a page descriptor(alloc new page during calling new_pd() )
  sc = (slab_cache *)alloc_slab((void *)sc_slab_tok);
  sc->head_pd = new_pd();
  sc->cache_pd = sc->head_pd;
  sc->page_slice_pos = sc->head_pd->page;
  sc->size = size;
  sc->free_count = 0;
  sc->page_remain = PAGE_SIZE;
  sc->next_slab = slab_st;
  slab_st = sc;
  uart_puts("new slab\n");
  return (void *)sc;
}

/*
 Kernel Memory Allocation
 size in byte
*/
void *kmalloc(unsigned long size) {
  uart_puts("(mm.c, kmalloc)\n");
  size = pad(size, 16);
  if (size > PAGE_SIZE / 2) {//if size > 2048B use allocate page
    return alloc_page(size);
  } 
  else {
    return alloc_slab(register_slab(size));//if size <= 2048B use allocate slab
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          kfree                                             // 
////////////////////////////////////////////////////////////////////////////////////////////////
/*
 free page & merge
 1. reset the buddy_stat
 2. keep merging buddy: reset the buddy_stat and pop the free_list of the buddy (註: buddy兩兩一對)
 3. set the topest buddy BUDDY_FREE and push into free_list
*/
void free_page(void *ptr) {
  uart_puts("free page: ");
  uart_put_hex((unsigned long)ptr);
  uart_puts("\n");
  
  // 1.
  // set buddy_stat
  unsigned long pagenum = ptr_to_pagenum(ptr);
  unsigned long ord = get_buddy_ord(buddy_stat[pagenum]);
  buddy_stat[pagenum] = INIT_PAGE;
  uart_puts("page pagenum: ");
  uart_put_hex(pagenum);
  uart_puts("\n");
  
  // 2.
  // try to merge from the lowest layer(4KB) to the topest layer(128MB)
  // if buddy is free, keep merging until buddy isn't free or buddy in other layers(different ord)
  while (ord < BUDDY_MAX - 1) {	// -1: BUDDY_MAX is the topest layer in buddy system
    unsigned long buddy = buddy_pagenum(pagenum, ord);
	uart_puts("order ");
    uart_put_int(ord);
	uart_puts(", buddy pagenum: ");
    uart_put_hex(buddy);
  	uart_puts("\n");
    if (get_buddy_flag(buddy_stat[buddy]) == BUDDY_FREE &&
        get_buddy_ord(buddy_stat[buddy]) == ord) {
      uart_puts("coalesce\n");
      pop_list((list_head *)pagenum_to_ptr(buddy));
      buddy_stat[buddy] = INIT_PAGE;
      ord++;
      pagenum = pagenum < buddy ? pagenum : buddy;	// idx of upper layer is smaller page num among two buddy
    } else {
      break;
    }
  }

  // 3.
  set_buddy_flag(buddy_stat[pagenum], BUDDY_FREE);
  set_buddy_ord(buddy_stat[pagenum], ord);
  push_list(&buddy.free_list[ord], pagenum_to_ptr(pagenum));
}

void free_reserve(void *ptr) {
  uart_puts("free reserve: ");
  uart_put_hex((unsigned long)ptr);
  uart_puts("\n");

  // find the end addr
  unsigned long st = (unsigned long)ptr;
  unsigned long ed;
  for (int i = 0; i < reserve_count; i++) {
    if ((unsigned long )startup.addr[i] == st) {
      ed = st + (unsigned long )startup.size[i];
    }
  }

  // 以下規則必須與place_buddy相同
  while (st != ed) {
    int st_ord = get_order(st);
    int ed_ord = get_order(ed);
    if (st_ord >= BUDDY_MAX) {
      st_ord = BUDDY_MAX - 1;
    }
    if (ed_ord >= BUDDY_MAX) {
      ed_ord = BUDDY_MAX - 1;
    }
    if (st_ord <= ed_ord) {
      free_page((void *)st);
      st += (1 << (st_ord + PAGE_SIZE_CTZ));
    } else {
      ed -= (1 << (ed_ord + PAGE_SIZE_CTZ));
      free_page((void *)ed);
    }
  }
}
void push_cache(cache_list **cl, cache_list *new_chunk) {
  new_chunk->next = (*cl);
  (*cl) = new_chunk;
}
void _free_slab(void *ptr, slab_cache *sc, page_descriptor *pd) {
  uart_puts("free slab inter\n");
  push_cache(&(pd->free_list), (cache_list *)ptr);
  pd->free_count++;
  sc->free_count++;
  if (sc->cache_pd->free_list == NULL) {
    sc->cache_pd = pd;
  }
}
void free_slab(void *ptr, void *slab) {
  uart_puts("free slab\n");
  page_descriptor *pd = ((slab_cache *)slab)->head_pd;
  while (pd != NULL) {
    if (ptr_to_pagenum(pd->page) == ptr_to_pagenum(ptr)) {
      _free_slab(ptr, ((slab_cache *)slab), pd);
      return;
    }
    pd = pd->next_pd;
  }
}
void free_unknow_slab(void *ptr) {
  uart_puts("free slab unknow\n");
  slab_cache *sc = slab_st;
  while (sc != NULL) {
    page_descriptor *pd = sc->head_pd;
    while (pd != NULL) {
      if (ptr_to_pagenum(pd->page) == ptr_to_pagenum(ptr)) {
        _free_slab(ptr, sc, pd);
        return;
      }
      pd = pd->next_pd;
    }
    sc = sc->next_slab;
  }
}
void kfree(void *ptr) {
  int flag = get_buddy_flag(buddy_stat[ptr_to_pagenum(ptr)]);
  if (flag == BUDDY_USE) {
    free_page(ptr);
  } else if (flag == RESRVE_USE) {
    free_reserve(ptr);
  } else if (flag == SLAB_USE) {
    free_unknow_slab(ptr);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////
//                                           print                                            // 
////////////////////////////////////////////////////////////////////////////////////////////////
void print_buddy_info(){
  uart_puts("*********************************************\n");
  uart_puts("buddy system\n");
  uart_puts("order\tfree_page\n");
  for (int i = 0; i < BUDDY_MAX; i++) {
    list_head *l = &buddy.free_list[i];
    list_head *head = l;
    int count = 0;
		uart_put_int(i);				// print order
		uart_puts("\t");
	  while (l->next != head) {
	    l = l->next;
	    count++;
	  }
		uart_put_int(count);
		uart_puts("\n");
  }
  uart_puts("*********************************************\n");
}
void print_buddy_stat(){
  uart_puts("*********************************************\n");
  for (unsigned long i = 0; i < mem_size / PAGE_SIZE; i++) {
    if (buddy_stat[i] != INIT_PAGE) {
		uart_puts("address :");
  		uart_put_hex(i);
  		uart_puts("\t");
  		uart_puts("state :");
  		uart_puts("\t");
      	if (get_buddy_flag(buddy_stat[i]) == BUDDY_FREE) {
        	uart_puts("buddy free");
      	} 
		else if (get_buddy_flag(buddy_stat[i]) == RESRVE_USE) {
        	uart_puts("reserve use");
      	} 
		else if (get_buddy_flag(buddy_stat[i]) == BUDDY_USE) {
        	uart_puts("buddy use");
      	} 
		else {
        	uart_puts("slab use");
      	}
      	uart_puts("\n");
    }
  }
  uart_puts("*********************************************\n");
}
void print_slab(){
	uart_puts("*********************************************\n");
	slab_cache *sc = slab_st;
	uart_puts("slab objects\n");
  	while (sc != NULL) {
		uart_puts("\t");
  		uart_put_int(sc->size);
  		uart_puts(" Bytes object: ");
		uart_puts("\tFree count: ");
		uart_put_int(sc->free_count);
		uart_puts("\tPage remain: ");
		uart_put_int(sc->page_remain);
		uart_puts("\n");
		sc = sc->next_slab; 
	}
	uart_puts("*********************************************\n");
}

char *demo_ptr1;
char *demo_ptr2;
void demo_alloc_page()
{
	demo_ptr1 = alloc_page(64*4096);
	demo_ptr2 = alloc_page(64*4096);
	print_buddy_info();
}
void demo_free_page()
{
	free_page(demo_ptr1);
	free_page(demo_ptr2);
	print_buddy_info();
}

char *demo_ptr3;
char *demo_ptr4;
void demo_kmalloc()
{
	demo_ptr3 = kmalloc(48);
	demo_ptr4 = kmalloc(21);
	print_slab();
}

void demo_kfree()
{
	kfree(demo_ptr3);
	print_slab();
}


void test_buddy(){
	print_buddy_info();
	char*p = alloc_page(64*4096);
	print_buddy_info();
	free_page(p);
	print_buddy_info();
}

void test_slab(){
	char* p1;
	print_slab();
	p1 = kmalloc(48);
	print_slab();
	kfree(p1);
	print_slab();
	p1 = kmalloc(128);
	print_slab();
	kfree(p1);
	print_slab();
}

