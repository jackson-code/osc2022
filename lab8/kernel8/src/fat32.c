#include "fat32.h"
#include "typedef.h"
#include "sd.h"
#include "my_string.h"
#include "mm.h"
#include "utils.h"

struct fat32_meta fat32_meta;

filesystem_t fat32;
file_op_t *fat32_f_op;
vnode_op_t *fat32_v_op;

int fat32_register() {
    fat32.name = "fat32";
    fat32.setup_mount = fat32_setup_mount;

    fat32_f_op = (file_op_t *)kmalloc(sizeof(file_op_t));
    fat32_f_op->open = fat32_open;
    fat32_f_op->close = fat32_close;
    fat32_f_op->read = fat32_read;
    fat32_f_op->write = fat32_write;

    fat32_v_op = (vnode_op_t *)kmalloc(sizeof(vnode_op_t));
    fat32_v_op->create = fat32_create;
    fat32_v_op->lookup = fat32_lookup;
    fat32_v_op->mkdir = fat32_mkdir;
    return 0;
}

int fat32_init() {
    register_filesystem("fat32");

    int ret;
    ret = vfs_mkdir("/boot");               // "/boot" is TA's spec
    ret = vfs_mount("/boot", "fat32");
    return ret;
}

int fat32_setup_mount(filesystem_t *tmpfs, struct mount *_mount) {
    // mount->fs = fat32;
    _mount->root->f_ops = fat32_f_op;
    _mount->root->v_ops = fat32_v_op;

    int parti_blk_idx = 2048;       // from TA's spec

    // 0. Get Partition
    char mbr[MBR_SIZE];
    read_classical_generic_MBR(mbr);
    // parse first partition only (have checked other partition are zero)
    struct mbr_partition_entry p1;
    memcpy(mbr + BOOTSTRAP_CODE_SIZE, &p1, sizeof(struct mbr_partition_entry));

    // 1.Parse the metadata on the SD card.
    // 2.Create a kernel object to store the metadata in memory.
    char buf[BLOCK_SIZE];
    // sd_read_block(p1.sector_beg, buf); // the boot sector is the first sector of a partition
    sd_read_block(parti_blk_idx, buf);             
    struct fat32_boot_sector bs;
    memcpy(buf, &bs, sizeof(struct fat32_boot_sector));

    // decide what info need later
    // size in sector
    fat32_meta.sec_size.reserved_sectors = bs.n_reserved_sectors;
    fat32_meta.sec_size.FAT_region = bs.n_FATs * bs.sectors_per_FAT;
    fat32_meta.sec_size.clus_size_byte = bs.bytes_per_logical_sector * bs.logical_sector_per_cluster;
    
    // sector begin
    fat32_meta.sec_beg.reserved_sectors = parti_blk_idx;
    fat32_meta.sec_beg.FAT_region = fat32_meta.sec_beg.reserved_sectors + fat32_meta.sec_size.reserved_sectors;
    fat32_meta.sec_beg.data_region = fat32_meta.sec_beg.FAT_region + fat32_meta.sec_size.FAT_region;
    fat32_meta.sec_beg.root_dir_entry
        = fat32_meta.sec_beg.data_region;


    // 3. Get the root directory cluster number and create it’s root directory object.    
    // int ret;
    // ret = vfs_mkdir("/boot");               // /boot is TA's spec
    // ret = vfs_mount("/boot", "fat32");
    // file_t *boot_dir;
    // vfs_open("/boot", O_CREAT, &boot_dir);
    // struct fat32_inter =  boot_dir->vnode->internal;


    // // dir table traversal (same code on fat32_lookup)
    // // if add new file into img, uncomment for checking new file existing or not
    // sd_read_block(fat32_meta.sec_beg.data_region, buf);
    // int offset = sizeof(struct fat32_dir_entry);
    // struct fat32_dir_entry *dir_entry = (struct fat32_dir_entry *)buf;
    // while (dir_entry->name[0] != FAT_DIR_TABLE_END)
    // {
    //     //ret = vfs_open((const char *)dir_entry->name, "fat32");
    //     offset += offset;
    //     dir_entry = (struct fat32_dir_entry *)(buf + offset);
    // }
    return 0;
}


//---------- vnode operation ----------//
void get_dir_entry_file_name(char *file_name, struct fat32_dir_entry *dir_entry)
{
    int i = 0, len = 0;
    while (dir_entry->name[i] != ' ' && i < 8)
    {
        *file_name++ = dir_entry->name[i++];
    }
    *file_name++ = '.';
    i = 0;
    while (dir_entry->ext[i] != ' ' && i < 3)
    {
        *file_name++ = dir_entry->ext[i++];
    }
    *file_name = '\0';
}

/*
    return the first cluster number of the file if success, otherwise return -1.
*/
int fat32_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* component_name) {
    // 1. Get the cluster number of the directory table and calculate its block index.
    // 2. Read the first block of the cluster.
    // 1. & 2. have done in fat32_setup_mount()
    char buf[BLOCK_SIZE];
    sd_read_block(fat32_meta.sec_beg.root_dir_entry, buf);
    struct fat32_dir_entry *dir_entry = (struct fat32_dir_entry *)buf;

    // 3. Traverse the directory table entries to find the file.
    int offset = sizeof(struct fat32_dir_entry);
    while (dir_entry->name[0] != FAT_DIR_TABLE_END)
    {
        // combine name and ext
        char *file_name;
        get_dir_entry_file_name(file_name, dir_entry);

        if (!str_cmp(file_name, component_name))      // find the file's dir entry
        {
            // You can get the first cluster number of the file in the directory table entry.
            return (dir_entry->clus_num_high << 16) | (dir_entry->clus_num_low);
        }
        // next entry
        offset += offset;
        dir_entry = (struct fat32_dir_entry *)(buf + offset);
    }
    return -1;
}



int fat32_create(vnode_t* dir_node, vnode_t** file_node, const char* file_name) {
    // if (!str_cmp("\0", (*file_node)->name))    // the file's vnode not existing
    // {
    //     // create file's vnode below dir_node 
    //     *file_node = tmpfs_create_vnode(file_name, dir_node, REGULAR_FILE);

    //     file_t *file = (file_t *)kmalloc(sizeof(file_t));
    //     return tmpfs_create_file(*file_node, &file); 
    // }
    // else
    // {
    //     uart_puts("\tERROR in fat32_create(): file existing");
    //     return -1; 
    // }
}
int fat32_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name) {

}
//---------- file operation ----------//
int fat32_write(file_t* file, const void* buf, unsigned long len) {

}
int fat32_read(file_t* file, void* buf, unsigned long len) {

}
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
int fat32_close(file_t* file) {

}