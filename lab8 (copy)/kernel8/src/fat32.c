#include "fat32.h"
#include "typedef.h"
#include "sd.h"
#include "my_string.h"

struct fat32_meta fat32_meta;

file_op_t *fat32_f_op;
vnode_op_t *fat32_v_op;

int fat32_register() {

    return 0;
}


//---------- vnode operation ----------//

/*
    return the first cluster number of the file if success, otherwise return -1.
*/
int fat32_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* component_name) {
    char buf[BLOCK_SIZE];

    // sd_read_block(fat32_meta.sec_beg.data_region, buf);

    // // dir table traversal 
    // int offset = sizeof(struct fat32_dir_entry);
    // struct fat32_dir_entry *dir_entry = (struct fat32_dir_entry *)buf;
    // while (dir_entry->name[0] != FAT_DIR_TABLE_END)
    // {
    //     //ret = vfs_open((const char *)dir_entry->name, "fat32");
    //     offset += offset;
    //     dir_entry = (struct fat32_dir_entry *)(buf + offset);
    // }


    // 1. Get the cluster number of the directory table and calculate its block index.
    // 2. Read the first block of the cluster.
    // 1. & 2. have done in sd_mount_fat32()
    sd_read_block(fat32_meta.sec_beg.root_dir_entry, buf);
    struct fat32_dir_entry *dir_entry = (struct fat32_dir_entry *)buf;

    // 3. Traverse the directory table entries to find the file.
    int offset = sizeof(struct fat32_dir_entry);
    while (dir_entry->name[0] != FAT_DIR_TABLE_END)
    {
        if (!str_cmp(dir_entry->name, component_name))      // find the file's dir entry
        {
            // You can get the first cluster number of the file in the directory table entry.
            return (dir_entry->clus_num_high << 16) | (dir_entry->clus_num_low >> 16);
        }
        // next entry
        offset += offset;
        dir_entry = (struct fat32_dir_entry *)(buf + offset);
    }
    return -1;
}

int fat32_create(vnode_t* dir_node, vnode_t** file_node, const char* file_name) {
    if (!str_cmp("\0", (*file_node)->name))    // the file's vnode not existing
    {
        // create file's vnode below dir_node 
        *file_node = tmpfs_create_vnode(file_name, dir_node, REGULAR_FILE);

        file_t *file = (file_t *)kmalloc(sizeof(file_t));
        return tmpfs_create_file(*file_node, &file); 
    }
    else
    {
        uart_puts("\tERROR in fat32_create(): file existing");
        return -1; 
    }
}
int fat32_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name);
//---------- file operation ----------//
int fat32_write(file_t* file, const void* buf, unsigned long len);
int fat32_read(file_t* file, void* buf, unsigned long len);
int fat32_open(vnode_t* file_node, file_t** target) {
    // if (file_node->type == REGULAR_FILE)
    // {
    //     return tmpfs_create_file(file_node, target); 
    // }
    // else
    // {
    //     uart_puts("\tERROR in tmpfs_open(): can't open directory vnode\n");
    //     return -1;
    // }
}
int fat32_close(file_t* file);