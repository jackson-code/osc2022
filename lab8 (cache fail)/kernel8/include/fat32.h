// Ref: en.wikipedia's Design of the FAT file system
#ifndef _FAT32_H_
#define _FAT32_H_

#include "typedef.h"
#include "vfs.h"

#define BLOCK_SIZE          (512)                       // TA's spec assign
#define FAT_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(int))
#define EOC                 (0xFFFFFF8)

//-------------------------------------------------//
//               classic generic MBR               //
//-------------------------------------------------//
#define MBR_SIZE                (512)
#define BOOTSTRAP_CODE_SIZE     (446)

struct mbr_partition_entry {
    uint8_t status_flag;                // 0x0
    uint8_t partition_beg_head;         // 0x1
    uint8_t partition_beg_sector;       // 0x2
    uint8_t partition_beg_cylinder;     // 0x3
    uint8_t partition_type;             // 0x4
    uint8_t partition_end_head;         // 0x5
    uint8_t partition_end_sector;       // 0x6
    uint8_t partition_end_cylinder;     // 0x7
    uint8_t sector_beg;                 // 0x8-0xB
    uint8_t number_of_sector;           // 0xC-0xF
};

void read_classical_generic_MBR(char *mbr);

struct fat32_boot_sector {
    char jump[3];  // 0x0
    char oem[8];   // 0x3

    // BIOS Parameter Block
    uint16_t bytes_per_logical_sector;   // 0xB-0xC
    uint8_t logical_sector_per_cluster;  // 0xD
    uint16_t n_reserved_sectors;         // 0xE-0xF
    uint8_t n_FATs;           // 0x10
    uint16_t n_max_root_dir_entries_16;  // 0x11-0x12
    uint16_t n_logical_sectors_16;       // 0x13-0x14
    uint8_t media_descriptor;            // 0x15
    uint16_t logical_sectors_per_FAT_for_fat16;  // 0x16-0x17, should be 0 for fat32

    // DOS3.31 BPB
    uint16_t physical_sector_per_track;  // 0x18-0x19
    uint16_t n_heads;                    // 0x1A-0x1B
    uint32_t n_hidden_sectors;           // 0x1C-0x1F
    uint32_t n_sectors_32;               // 0x20-0x23

    // FAT32 Extended BIOS Parameter Block
    uint32_t sectors_per_FAT;                    // 0x24-0x27
    uint16_t drive_description;                 // 0x28-0x29 (mirror_flag)
    uint16_t version;                           // 0x2A-0x2B
    uint32_t clus_num_of_root_dir;              // 0x2C-0x2F, cluster num of root directory table
    uint16_t fs_info_sector_num;                // 0x30-0x31
    uint16_t boot_sector_bak_first_sector_num;  // 0x32-0x33
    uint32_t reserved[3];                       // 0x34-0x3F
    uint8_t physical_drive_num;                 // 0x40
    uint8_t unused;                             // 0x41
    uint8_t extended_boot_signature;            // 0x42
    uint32_t volume_id;                         // 0x43-0x46
    uint8_t volume_label[11];                   // 0x47-0x51
    uint8_t fat_system_type[8];                 // 0x52-0x59
} __attribute__((packed));

//----- directory table -----//
#define FAT_DIR_TABLE_END                       (0x00)

#define FAT_DIR_ENTRY_ATTR                      (0x20)

struct fat32_dir_entry {
    uint8_t short_name[8];                // 0x0-0x7
    uint8_t ext[3];                 // 0x8-0xA
    uint8_t attr;                   // 0xB
    uint8_t reserved;               // 0xC
    uint8_t create_time[3];         // 0xD-0xF
    uint16_t create_date;           // 0x10-0x11
    uint16_t last_access_date;      // 0x12-0x13
    uint16_t clus_num_high;         // 0x14-0x15
    uint32_t ext_attr;              // 0x16-0x19
    uint16_t clus_num_low;          // 0x1A-0x1B
    uint32_t file_size;             // 0x1C-0x1F, size in bytes
} __attribute__((packed));
//---------------------------//



// unit in sectors
struct fat32_layout_size {
    uint32_t reserved_sectors;
    uint32_t FAT_region;
};

// unit in sectors
struct fat32_sector_beg {
    uint32_t reserved_sectors;
    uint32_t FAT_region;
    uint32_t data_region;
    uint32_t root_dir_entry;
};

struct fat32_meta {
    struct fat32_layout_size sec_size;
    struct fat32_sector_beg sec_beg;
    uint32_t first_clus;
    uint32_t eoccm;                         // end of cluster chain marker
    vnode_t *root;
};

enum cache_status {
    DIRTY = 1,
    CLEAR = 2,
    EMPTY = 3,
};

typedef struct fat32_internal {
    uint32_t first_cluster;
    // uint32_t dirent_cluster;
    // uint32_t size;

    // cache for meta data
    uint32_t *FAT;
    uint32_t FAT_blk_idx;
    struct fat32_dir_entry *dir_table;
    uint32_t dir_table_blk_idx;
    enum cache_status meta_status;
    // cache for file content
    // char *file_content;
    // struct file_cache file_cache; 
    char *file_content;
    enum cache_status file_status;
    uint32_t file_blk_idx;
}inter_fat32_t;

struct file_cache {
    char *file_content;
    enum cache_status file_status;
    uint32_t file_blk_idx;
    // uint32_t size;
};

int fat32_init();
int fat32_register();
int fat32_setup_mount(filesystem_t *tmpfs, struct mount *mount);
vnode_t *fat32_create_vnode(const char *name, vnode_t *parent, enum vnode_type type, uint32_t first_clus);
//---------- file operation ----------//
int fat32_write(file_t* file, const void* buf, unsigned long len);
int fat32_read(file_t* file, void* buf, unsigned long len);
int fat32_open(vnode_t* file_node, file_t** target);
int fat32_close(file_t* file);
//---------- vnode operation ----------//
int fat32_lookup(vnode_t* dir_node, vnode_t** target, const char* component_name);
int fat32_create(vnode_t* dir_node, vnode_t** target, const char* component_name);
int fat32_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name);



// memory cached SD card
struct file_meta {

    // struct list_head next;
};




#endif