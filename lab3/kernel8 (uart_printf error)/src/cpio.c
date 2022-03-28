// ref: FreeBSD Manual Pages cpio
#include "cpio.h"
#include "uart.h"
#include "my_string.h"
#include "convert.h"


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
		uart_printf("%s", pathname);
		
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
        uart_printf("Can't find the file: %s", args);
    }
}
