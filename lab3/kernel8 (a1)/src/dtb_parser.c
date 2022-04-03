#include "dtb_parser.h"

void dt_info(){
    unsigned long *dt_addr = (unsigned long *)DT_ADDR;
    struct fdt_header* header = (struct fdt_header *)*dt_addr;

    unsigned int totalsize, off_mem_rsvmap, off_dt_struct, off_dt_strings, size_dt_strings, size_dt_struct;
    totalsize = btol(header->totalsize);
    off_dt_struct = btol(header->off_dt_struct);
    off_dt_strings = btol(header->off_dt_strings);
    off_mem_rsvmap = btol(header->off_mem_rsvmap);
    size_dt_strings = btol(header->size_dt_strings);
    size_dt_struct = btol(header->size_dt_struct);

    uart_printf("Device Tree Address\t%d", *dt_addr);
    uart_printf("Total Size\t%d", totalsize);
    uart_printf("off_mem_rsvmap\t%d", off_mem_rsvmap);
    uart_printf("off_dt_struct\t%d", off_dt_struct);
    uart_printf("size_dt_struct\t%d", size_dt_struct);
    uart_printf("off_dt_strings\t%d", off_dt_strings);
    uart_printf("size_dt_strings\t%d", size_dt_strings);
}

void dt_parse(){
    unsigned long *dt_addr = (unsigned long *)DT_ADDR;
    struct fdt_header* header = (struct fdt_header *)*dt_addr;

    unsigned int off_dt_struct, off_dt_strings, size_dt_struct;
    off_dt_struct = btol(header->off_dt_struct);
    off_dt_strings = btol(header->off_dt_strings);
    size_dt_struct = btol(header->size_dt_struct);
    unsigned long struct_addr = *dt_addr + off_dt_struct;
    unsigned long string_addr = *dt_addr + off_dt_strings;
    unsigned long struct_end_addr = struct_addr + size_dt_struct;
    unsigned int *struct_ptr = (unsigned int*)struct_addr;
	
    unsigned int cur_val, cur_len, cur_nameoff;
    char *cur_name, *cur_name_val;
	
    while((unsigned long)struct_ptr < struct_end_addr){
        cur_val = btol(*struct_ptr);
        struct_ptr += 1;

        if(cur_val == FDT_BEGIN_NODE){
            cur_name = (char*)struct_ptr;
            uart_printf("------------------------------------------------");
            uart_printf(cur_name);
            cur_len = str_len(cur_name);
            struct_ptr += cur_len/4;
            struct_ptr += (cur_len%4 ? 1 : 0);
        }
        else if(cur_val == FDT_END_NODE){
            uart_printf("------------------------------------------------");
        }
        else if(cur_val == FDT_PROP){
            cur_len = btol(*struct_ptr);
            struct_ptr += 1;
            cur_nameoff = btol(*struct_ptr);
            struct_ptr += 1;
        
            if(cur_len != 0){   // Not Empty Property
                cur_name = (char*)string_addr+cur_nameoff;
                cur_name_val = (char*)struct_ptr;
                struct_ptr += cur_len/4;
                struct_ptr += (cur_len%4 ? 1 : 0);
                uart_printf("Len: %d", cur_len);
                uart_printf("Cur Name: %s", cur_name);
                uart_printf("Cur Value: %s", cur_name_val);
                uart_printf("------------------------------------------------");
            }
        }
        else if(cur_val == FDT_NOP){
            ;
        }
        else if(cur_val == FDT_END){
            break;
        }
		else if(cur_val == PADDING){
            ;
        }
        else{
            uart_printf("ERROR! Unknown Node Value: %d", cur_val);
            return;
        }
    }
}
