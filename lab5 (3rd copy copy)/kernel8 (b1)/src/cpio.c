// ref: FreeBSD Manual Pages cpio
#include "cpio.h"
#include "uart.h"
#include "my_string.h"
#include "convert.h"

char *CPIO_ADDR = 0;
extern int property_qemu;     

void decide_cpio_addr()
{
    if (property_qemu)
        CPIO_ADDR = ((char*)0x8000000);      // QEMU(0x8000000)
    else {
        // [NOTE!!!] have to same with mm.c: init_startup(), and config.txt
        CPIO_ADDR = ((char*)0x20000000); 
    }
}

void cpio_init()
{
    decide_cpio_addr();
}

// calculate how many bytes have to padding
// multiplier always 4 in New ASCII Format
unsigned long  align(unsigned long  size, unsigned long  multiplier){
    if(multiplier <= 0) return 0;
    else return (multiplier - (size % multiplier)) % multiplier;	// last % multiplier is for the case: size = n * multiplier
}

void get_size_info(struct cpio_new_header *cpio_addr, struct cpio_size_info *size_info){
    size_info->file_size = ahtoi(cpio_addr->c_filesize, 8);			// each filed uses 8-byte hexadecimal in New ASCII Format
    size_info->file_align = align(size_info->file_size, 4);
    size_info->name_size = ahtoi(cpio_addr->c_namesize, 8);
    size_info->name_align = align(size_info->name_size + CPIO_HEADER_SIZE, 4);
    size_info->offset = CPIO_HEADER_SIZE + size_info->file_size + size_info->file_align + size_info->name_size + size_info->name_align;
}

void cpio_list(){
	char *cur_addr_byte = CPIO_ADDR;														// addr in bytes
	struct cpio_new_header *cur_addr_struct = (struct cpio_new_header *)cur_addr_byte;		// addr in struct
	struct cpio_size_info size_info;
	
	// go through all cpio archive, get all pathname
	while (1) {
		char *pathname = (char *)(cur_addr_struct + 1);
		if (!str_cmp("TRAILER!!!", pathname)) break;		// end of the archive
		uart_puts(pathname);
		
		// get total length of the file
		get_size_info(cur_addr_struct, &size_info);
		// point to next file
		cur_addr_byte += size_info.offset;
		cur_addr_struct = (struct cpio_new_header *)cur_addr_byte;
	}
}

void cpio_cat(char *args){
    char *cur_addr_byte = CPIO_ADDR;													// addr in byte
    struct cpio_new_header *cur_addr_struct = (struct cpio_new_header* )cur_addr_byte;	// addr in struct
    struct cpio_size_info size_info;
    
    int file_missing = 1;
    while (1) {
    	char *pathname = (char*)((char*)cur_addr_struct + CPIO_HEADER_SIZE);
    	if (!str_cmp("TRAILER!!!", pathname)) 
    		break;
    		
        get_size_info(cur_addr_struct, &size_info);        
        // print file content
        if (!str_cmp(args, pathname)) {
            uart_puts_bySize((char*)((char*)cur_addr_struct + CPIO_HEADER_SIZE + size_info.name_size + size_info.name_align), 									size_info.file_size);
            file_missing = 0;
        }
        
        // point to next file
        cur_addr_byte += size_info.offset;
        cur_addr_struct = (struct cpio_new_header* )cur_addr_byte;
    }
    
    if(file_missing){
        uart_puts("Can't find the file");
    }
}


// get file address
// retrun "no file\0" if can't find the file
char* cpio_get_addr(char *args){
	uart_puts("cpio_get_addr() begin\n");

    char *cur_addr_byte = CPIO_ADDR;													// addr in byte
    struct cpio_new_header *cur_addr_struct = (struct cpio_new_header* )cur_addr_byte;	// addr in struct
    struct cpio_size_info size_info;

    while (1) {
    	char *pathname = (char*)((char*)cur_addr_struct + CPIO_HEADER_SIZE);
    	if (!str_cmp("TRAILER!!!", pathname)) 
    		break;

        //uart_puts("\t finding file\n");	
        //uart_puts(pathname);
        //uart_puts("\n");	
        
        get_size_info(cur_addr_struct, &size_info);       
        
        if (!str_cmp(args, pathname)) {
        	// get addr of file
            uart_puts("\t find the file!!!\n");	
            return (char*)((char*)cur_addr_struct + CPIO_HEADER_SIZE + size_info.name_size + size_info.name_align);
        }
        
        // point to next file
        cur_addr_byte += size_info.offset;
        cur_addr_struct = (struct cpio_new_header* )cur_addr_byte;
    }
    
    // file_missing
    uart_puts("Can't find the file\n");
    return "no file\0";
}

int find_app_size(char* target){
    char *now_ptr = CPIO_ADDR;
    struct cpio_new_header *cpio_addr = (struct cpio_new_header* )now_ptr;
    struct cpio_size_info size_info;
	while(1){
        get_size_info(cpio_addr, &size_info);
        char *pathname = (char*)((char*)cpio_addr + CPIO_HEADER_SIZE);
        if(str_cmp("TRAILER!!!", pathname) == 0) break;
        if(str_cmp(target, pathname) == 0){
			//uart_puts("CPIO address :");
			//uart_put_hex((unsigned long)cpio_addr);
			//uart_puts("\r\n\r\n");
            //uart_puts_bySize((char*)((char*)cpio_addr + CPIO_SIZE + size_info.name_size + size_info.name_align), size_info.file_size);
            //uart_puts("\r\n");
            return size_info.file_size;
        }
        now_ptr += size_info.offset;
        cpio_addr = (struct cpio_new_header* )now_ptr;
	}
	return 0;
}
