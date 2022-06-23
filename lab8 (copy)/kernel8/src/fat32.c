#include "fat32.h"
#include "typedef.h"
#include "sd.h"
#include "my_string.h"
#include "mm.h"
#include "utils.h"
#include "uart.h"

struct fat32_meta fat32_meta;

filesystem_t fat32;
file_op_t *fat32_f_op;
vnode_op_t *fat32_v_op;

void get_complete_file_name(char *file_name, struct fat32_dir_entry *dir_entry)
{
    int i = 0;
    while (dir_entry->short_name[i] != ' ' && i < 8)
    {
        *file_name++ = dir_entry->short_name[i++];
    }
    *file_name++ = '.';
    i = 0;
    while (dir_entry->ext[i] != ' ' && i < 3)
    {
        *file_name++ = dir_entry->ext[i++];
    }
    *file_name = '\0';
}

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

    vfs_mkdir("/boot");               // "/boot" is TA's spec
    vfs_mount("/boot", "fat32");

    return 0;
}

int fat32_setup_mount(filesystem_t *tmpfs, struct mount *_mount) {
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
    // fat32_meta.sec_size.clus_size_byte = bs.bytes_per_logical_sector * bs.logical_sector_per_cluster;
    
    // sector begin
    fat32_meta.sec_beg.reserved_sectors = parti_blk_idx;
    fat32_meta.sec_beg.FAT_region = fat32_meta.sec_beg.reserved_sectors + fat32_meta.sec_size.reserved_sectors;
    fat32_meta.sec_beg.data_region = fat32_meta.sec_beg.FAT_region + fat32_meta.sec_size.FAT_region;
    fat32_meta.sec_beg.root_dir_entry
        = fat32_meta.sec_beg.data_region;

    fat32_meta.first_clus = bs.clus_num_of_root_dir;
    // FAT
    uint32_t FAT[BLOCK_SIZE / sizeof(uint32_t)];
    sd_read_block(fat32_meta.sec_beg.FAT_region, FAT);
    fat32_meta.eoccm = *(FAT + 1);    // end of cluster chain marker

    fat32_meta.root = _mount->root;

    return 0;
}


//---------- vnode operation ----------//
/*
    return the first cluster number of the file if success, otherwise return -1.
*/
int fat32_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* component_name) {
    // finding vnode in dir_node's child (componen name cache mechanism)
    for (list_head *sibling = dir_node->subdirs.next; sibling != &dir_node->subdirs; sibling = sibling->next)
    {
        vnode_t *child = list_entry(sibling, struct vnode, siblings);
        if (!str_cmp(child->name, component_name))
        {
            *v_tar = child;
            return 0;
        }
    }
 
    // finding in dir table, create vnode for the dir entry
    struct fat32_dir_entry dir_table[BLOCK_SIZE / sizeof(struct fat32_dir_entry)];
    sd_read_block(fat32_meta.sec_beg.data_region, dir_table);
    struct fat32_dir_entry *dir_entry = dir_table;
    while (dir_entry->short_name[0] != FAT_DIR_TABLE_END)
    {
        // combine name and ext
        char *file_name = (char *)kmalloc(sizeof(char) * (8+1+3+1));
        get_complete_file_name(file_name, dir_entry);

        if (!str_cmp(file_name, component_name))
        {
            *v_tar = fat32_create_vnode(file_name, fat32_meta.root, REGULAR_FILE, (dir_entry->clus_num_high << 16) | (dir_entry->clus_num_low));
            return 0;
        }
        dir_entry++;
    }
    
    return -1;
}

vnode_t *fat32_create_vnode(const char *name, vnode_t *parent, enum vnode_type type, uint32_t first_clus) {
    vnode_t *v = (vnode_t *)kmalloc(sizeof(vnode_t));
    if (parent != 0)
    {
        v->mount = parent->mount;
    }
    else
    {
        v->mount = rootfs;
    }
    v->f_ops = fat32_f_op;
    v->v_ops = fat32_v_op;
    inter_fat32_t *inter = (inter_fat32_t *)kmalloc(sizeof(inter_fat32_t));
    inter->first_cluster = first_clus;
    inter->file_cache->status = EMPTY;
    v->internal = inter;

    v->file = 0;
    v->parent = parent;
    str_cpy(v->name, name);
    v->type = type;
    list_init(&v->siblings);
    list_init(&v->subdirs);
    if (parent != 0)
    {
        list_push(&parent->subdirs, &v->siblings);
    }
    uart_puts("fat32_create_vnode():\n\tvnode at 0x");
    uart_put_hex((unsigned long)v);
    if (parent != 0) {
        uart_puts("\tparent->subdirs at 0x");
        uart_put_hex((unsigned long)&parent->subdirs);
    }
    uart_puts("\tv->siblings at 0x");
    uart_put_hex((unsigned long)&v->siblings);
    uart_puts(",\tsubdirs at 0x");
    uart_put_hex((unsigned long)&v->subdirs);
    uart_puts("\n");
    return v;
}

int fat32_create_file(vnode_t* file_node, file_t** target) {
    if (file_node->file == 0 || (*target)->status == FILE_NOT_EXIST)   // file not existing
    {
        // create file
        *target = (file_t *)kmalloc(sizeof(file_t));
        (*target)->vnode = file_node;
        (*target)->f_ops = file_node->f_ops;
        (*target)->f_pos = 0;
        (*target)->size = 0;
        (*target)->status = FILE_EXIST;

        file_node->file = *target;

        // // won't into the if, if the file wasn't opened first time
        // if (file_node->internal == 0)   // first time to establish the file
        // {
        //     // alloc space for file's content
        //     inter_tmpfs_t *inter = (inter_tmpfs_t *)kmalloc(sizeof(inter_tmpfs_t));
        //     inter->file_content = (char *)kmalloc(TMPFS_MAX_FILE_SIZE);
        //     file_node->internal = inter;
        // }

        return 0;
    }
    else
    {
        *target = file_node->file;
        uart_puts("\tWARNING tmpfs_create_file(): file existing\n");
        return 0;
    }
}

// TODO
int fat32_create(vnode_t* dir_node, vnode_t** file_node, const char* file_name) {
    if (!str_cmp("\0", (*file_node)->name))    // the file's vnode not existing
    {
        // 1. Find an empty entry in the FAT table.
        uint32_t table_idx = 0;
        uint32_t table_size = BLOCK_SIZE / sizeof(uint32_t);
        uint32_t *FAT = (uint32_t *)kmalloc(table_size * sizeof(uint32_t));
        // uint32_t FAT[table_size];
        uint32_t first_clus = 2;                 // skip special file
        while (first_clus != 0)
        {
            if (table_idx > 0)
                first_clus = 0;                 

            sd_read_block(fat32_meta.sec_beg.FAT_region + table_idx, FAT);
            // find the first empty entry in FAT
            while (first_clus < table_size && *(FAT + first_clus) != 0) {
                first_clus++;
            }
            table_idx++;
        }
        *(FAT + first_clus) = fat32_meta.eoccm;
        sd_write_block(fat32_meta.sec_beg.FAT_region + table_idx, FAT);
        first_clus += table_size * table_idx;

        // 2. Find an empty directory entry in the target directory.
        struct fat32_dir_entry *dir_table = (struct fat32_dir_entry *)kmalloc(BLOCK_SIZE);
        // struct fat32_dir_entry dir_table[BLOCK_SIZE / sizeof(struct fat32_dir_entry)];
        sd_read_block(fat32_meta.sec_beg.data_region, dir_table);
        struct fat32_dir_entry *dir_entry = dir_table;
        while (dir_entry->short_name[0] != FAT_DIR_TABLE_END)
        {
            dir_entry++;
        }

        // get file name & ext (TODO: free comp_names)
        int comp_count = str_token_count(file_name, '.');
        char *comp_names[comp_count];
        int max_name_len = 64;
        for (int j = 0; j < comp_count; j++) {
            comp_names[j] = (char *)kmalloc(max_name_len);
        }
        str_token(file_name, comp_names, '.');

        // fill file name with space
        int i = str_len(comp_names[0]);
        while (i < 8)
        {
            *(comp_names[0] + i) = ' ';
            i++;
        }
        *(comp_names[0] + i) = '\0';

        // set dir entry
        str_cpy((char *)dir_entry->short_name, comp_names[0]);
        str_cpy((char *)dir_entry->ext, comp_names[1]);
        dir_entry->attr = FAT_DIR_ENTRY_ATTR;
        dir_entry->clus_num_high = first_clus >> 16;                 // 0x14-0x15
        dir_entry->clus_num_low = (first_clus << 16) >> 16;          // 0x1A-0x1B
        sd_write_block(fat32_meta.sec_beg.data_region, dir_table);

        // create file's vnode below dir_node 
        *file_node = fat32_create_vnode(file_name, dir_node, REGULAR_FILE, first_clus);
        file_t *file;
        fat32_create_file(*file_node, &file); 

        // cache 
        inter_fat32_t *inter = (inter_fat32_t*)((*file_node)->internal);
        inter->FAT = FAT;
        inter->dir_table = dir_table;

        return 0;
    }
    else
    {
        uart_puts("\tERROR in fat32_create(): file existing");
        return -1; 
    }
}
int fat32_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name) {
    return -1;
}

// TODO
int update_dir_entry_size(uint32_t size, const char *name, vnode_t *v) {
    // traverse dir table
    // struct fat32_dir_entry dir_table[BLOCK_SIZE / sizeof(struct fat32_dir_entry)];
    // sd_read_block(fat32_meta.sec_beg.data_region, dir_table);
    struct fat32_dir_entry *dir_table = ((inter_fat32_t *)(v->internal))->dir_table;
    struct fat32_dir_entry *dir_entry = dir_table;

    // combine name and ext
    char *file_name = (char *)kmalloc(sizeof(char) * (8+1+3+1));
    get_complete_file_name(file_name, dir_entry);

    // finding target
    while (dir_entry->short_name[0] != FAT_DIR_TABLE_END && str_cmp(file_name, name) != 0)
    {
        dir_entry++;
        get_complete_file_name(file_name, dir_entry);
    }

    // find target
    if (dir_entry->short_name[0] != FAT_DIR_TABLE_END)
    {
        dir_entry->file_size = size;
        sd_write_block(fat32_meta.sec_beg.data_region, dir_table);
        return 0;
    }
    return -1;
}


//---------- file operation ----------//
// TODO
int fat32_write(file_t* file, const void* buf, unsigned long len) {
    inter_fat32_t *inter = (inter_fat32_t *)file->vnode->internal;
    
    char *src = (char *)buf;
    char *des;
    // sd_read_block(fat32_meta.sec_beg.data_region + inter->first_cluster - fat32_meta.first_clus, des);

    if (inter->file_cache->status == EMPTY)
    {
        des = (char *)kmalloc(BLOCK_SIZE);
        inter->file_cache->content = des;
        sd_read_block(fat32_meta.sec_beg.data_region + inter->first_cluster - fat32_meta.first_clus, des);
    } else {
        des = inter->file_cache->content;
    }

    unsigned i = file->f_pos;
    while (len > 0 && *src != '\0')
    {
        des[i++] = *src++;
        len--;
    }
    des[i] = '\0';

    sd_write_block(fat32_meta.sec_beg.data_region + inter->first_cluster - fat32_meta.first_clus, des);

    inter->file_cache->status = DIRTY;

    // uart_puts((char *)buf);
    // uart_puts("\n");
    // uart_puts((char *)des);
    // uart_puts("\n");

    unsigned write_size = i - file->f_pos;
    file->f_pos += write_size;
    file->size += write_size;

    update_dir_entry_size(file->size, file->vnode->name, file->vnode);

    return write_size;
}
// TODO (OK)
int fat32_read(file_t* file, void* buf, unsigned long len) {
    inter_fat32_t *inter = (inter_fat32_t *)file->vnode->internal;
    
    char *des = (char *)buf;
    char *src;

    if (inter->file_cache->status == EMPTY)
    {
        src = (char *)kmalloc(BLOCK_SIZE);
        inter->file_cache->content = src;
        inter->file_cache->status = CLEAR;
        sd_read_block(fat32_meta.sec_beg.data_region + inter->first_cluster - fat32_meta.first_clus, src);
    } else {
        src = inter->file_cache->content;
    }

    unsigned i = file->f_pos;
    while (len > 0 && src[i] != '\0')
    {
        *des++ = src[i++];
        len--;
    }
    
    uart_puts((char *)buf);
    uart_puts("\n");

    unsigned read_size = i - file->f_pos;
    file->f_pos += read_size;

    return read_size;
}

int fat32_open(vnode_t* vnode, file_t** target) {
    if (vnode->type == REGULAR_FILE)
    {
        return fat32_create_file(vnode, target); 
    }
    else
    {
        uart_puts("\tERROR in fat32_open(): can't open directory vnode\n");
        return -1;
    }
}
int fat32_close(file_t* file) {
    file->status = FILE_NOT_EXIST;
    file->vnode->file = 0;
    kfree(file);
    return 0;
}









    // uint32_t FAT_blk_count = fat32_meta.sec_size.FAT_region;
    // uint32_t FAT_entry_count =  BLOCK_SIZE / sizeof(uint32_t);
    // uint32_t FAT_total_entry_count = FAT_blk_count * FAT_entry_count;
    // uint32_t *FAT = (uint32_t *)kmalloc(fat32_meta.sec_size.FAT_region * bs->bytes_per_logical_sector);
    // // uint32_t FAT[fat32_meta.sec_size.FAT_region * bs->bytes_per_logical_sector / sizeof(uint32_t)];
    // // uint32_t FAT[FAT_toal_entry_count];

    // // load the whole FAT into local variable FAT
    // uint32_t blk_idx = 0;
    // while (blk_idx < FAT_blk_count)
    // {
    //     sd_read_block(fat32_meta.sec_beg.FAT_region + blk_idx, FAT + blk_idx * BLOCK_SIZE / sizeof(uint32_t));
    //     blk_idx++;
    //     uart_put_int(blk_idx);
    // }
    // uart_puts("\nOK~~~\n");
    // blk_idx = 0;

    // // int32_t seen[FAT_total_entry_count];
    // int32_t *seen = (int32_t *)kmalloc(FAT_total_entry_count);
    // // uint32_t clus_sets[FAT_total_entry_count];
    // uint32_t *clus_sets = (uint32_t *)kmalloc(FAT_total_entry_count);
    // uint32_t clus_idx = 0;

    // // special entries (the first two entries in whole FAT region)
    // uint32_t EOCCM = *(FAT + 1); // end of cluster chain marker
    
    // // // skip special entry
    // // uint32_t entry_idx = 2;                                 
    // uint32_t seen_idx = 2;                                 
    // seen[0] = seen[1] = -1;                                 // -1 means the extry have seen
 
    // uint32_t cur_entry = 2;                                 // skip special entry
    // uint32_t next_entry;
    // uint32_t count = 0;
    // // while (seen_idx < FAT_total_entry_count)
    // while (count < FAT_total_entry_count)
    // {
    //     count++;
    //     // uart_put_int(seen_idx);
    //     // uart_puts(" ");
    //     //ret = vfs_open((const char *)dir_entry->name, "fat32");

    //     if (cur_entry == EOCCM || cur_entry == EOC)
    //     {
    //         clus_sets[clus_idx++] = 0;    // use 0 as delimiter between file's clusters
            
    //         // find unseen clus num
    //         while (seen[seen_idx] == -1 || *(FAT + seen_idx) == 0)
    //         {
    //             seen_idx++;
    //         }
    //         next_entry = seen_idx;         
    //     }
    //     else if (cur_entry != 0)
    //     {
    //         seen[cur_entry] = -1;
    //         next_entry = *(FAT + cur_entry);
            
    //         clus_sets[clus_idx++] = cur_entry;
    //         // clusters_size++;
    //     }
    //     cur_entry = next_entry;
    // }