#include "sd.h"
#include "uart.h"
#include "utils.h"
#include "vfs.h"
#include "mm.h"
#include "fat32.h"

// helper
#define set(io_addr, val) \
    asm volatile("str %w1, [%0]" ::"r"(io_addr), "r"(val) : "memory");

#define get(io_addr, val) \
    asm volatile("ldr %w0, [%1]" : "=r"(val) : "r"(io_addr) : "memory");

static inline void delay(unsigned long tick) {
    while (tick--) {
        asm volatile("nop");
    }
}

static int is_hcs;  // high capcacity(SDHC)

static void pin_setup() {
    set(GPIO_GPFSEL4, 0x24000000);
    set(GPIO_GPFSEL5, 0x924);
    set(GPIO_GPPUD, 0);
    delay(15000);
    set(GPIO_GPPUDCLK1, 0xffffffff);
    delay(15000);
    set(GPIO_GPPUDCLK1, 0);
}

static void sdhost_setup() {
    unsigned int tmp;
    set(SDHOST_PWR, 0);
    set(SDHOST_CMD, 0);
    set(SDHOST_ARG, 0);
    set(SDHOST_TOUT, SDHOST_TOUT_DEFAULT);
    set(SDHOST_CDIV, 0);
    set(SDHOST_HSTS, SDHOST_HSTS_MASK);
    set(SDHOST_CFG, 0);
    set(SDHOST_CNT, 0);
    set(SDHOST_SIZE, 0);
    get(SDHOST_DBG, tmp);
    tmp &= ~SDHOST_DBG_MASK;
    tmp |= SDHOST_DBG_FIFO;
    set(SDHOST_DBG, tmp);
    delay(250000);
    set(SDHOST_PWR, 1);
    delay(250000);
    set(SDHOST_CFG, SDHOST_CFG_SLOW | SDHOST_CFG_INTBUS | SDHOST_CFG_DATA_EN);
    set(SDHOST_CDIV, SDHOST_CDIV_DEFAULT);
}

static int wait_sd() {
    int cnt = 1000000;
    unsigned int cmd;
    do {
        if (cnt == 0) {
        return -1;
        }
        get(SDHOST_CMD, cmd);
        --cnt;
    } while (cmd & SDHOST_NEW_CMD);
    return 0;
}

static int sd_cmd(unsigned cmd, unsigned int arg) {
    set(SDHOST_ARG, arg);
    set(SDHOST_CMD, cmd | SDHOST_NEW_CMD);
    return wait_sd();
}

static int sdcard_setup() {
    unsigned int tmp;
    sd_cmd(GO_IDLE_STATE | SDHOST_NO_REPONSE, 0);
    sd_cmd(SEND_IF_COND, VOLTAGE_CHECK_PATTERN);
    get(SDHOST_RESP0, tmp);
    if (tmp != VOLTAGE_CHECK_PATTERN) {
        return -1;
    }
    while (1) {
        if (sd_cmd(APP_CMD, 0) == -1) {
        // MMC card or invalid card status
        // currently not support
        continue;
        }
        sd_cmd(SD_APP_OP_COND, SDCARD_3_3V | SDCARD_ISHCS);
        get(SDHOST_RESP0, tmp);
        if (tmp & SDCARD_READY) {
        break;
        }
        delay(1000000);
    }

    is_hcs = tmp & SDCARD_ISHCS;
    sd_cmd(ALL_SEND_CID | SDHOST_LONG_RESPONSE, 0);
    sd_cmd(SEND_RELATIVE_ADDR, 0);
    get(SDHOST_RESP0, tmp);
    sd_cmd(SELECT_CARD, tmp);
    sd_cmd(SET_BLOCKLEN, 512);
    return 0;
}

static int wait_fifo() {
    int cnt = 1000000;
    unsigned int hsts;
    do {
        if (cnt == 0) {
        return -1;
        }
        get(SDHOST_HSTS, hsts);
        --cnt;
    } while ((hsts & SDHOST_HSTS_DATA) == 0);
    return 0;
}

static void set_block(int size, int cnt) {
    set(SDHOST_SIZE, size);
    set(SDHOST_CNT, cnt);
}

static void wait_finish() {
    unsigned int dbg;
    do {
        get(SDHOST_DBG, dbg);
    } while ((dbg & SDHOST_DBG_FSM_MASK) != SDHOST_HSTS_DATA);
}

void sd_read_block(int block_idx, void* buf) {
    unsigned int* buf_u = (unsigned int*)buf;
    int succ = 0;
    if (!is_hcs) {
        block_idx <<= 9;
    }
    do{
        set_block(512, 1);
        sd_cmd(READ_SINGLE_BLOCK | SDHOST_READ, block_idx);
        for (int i = 0; i < 128; ++i) {
        wait_fifo();
        get(SDHOST_DATA, buf_u[i]);
        }
        unsigned int hsts;
        get(SDHOST_HSTS, hsts);
        if (hsts & SDHOST_HSTS_ERR_MASK) {
        set(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
        sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
        } else {
        succ = 1;
        }
    } while(!succ);
    wait_finish();
}

void sd_write_block(int block_idx, void* buf) {
    unsigned int* buf_u = (unsigned int*)buf;
    int succ = 0;
    if (!is_hcs) {
        block_idx <<= 9;
    }
    do{
        set_block(512, 1);
        sd_cmd(WRITE_SINGLE_BLOCK | SDHOST_WRITE, block_idx);
        for (int i = 0; i < 128; ++i) {
        wait_fifo();
        set(SDHOST_DATA, buf_u[i]);
        }
        unsigned int hsts;
        get(SDHOST_HSTS, hsts);
        if (hsts & SDHOST_HSTS_ERR_MASK) {
        set(SDHOST_HSTS, SDHOST_HSTS_ERR_MASK);
        sd_cmd(STOP_TRANSMISSION | SDHOST_BUSY, 0);
        } else {
        succ = 1;
        }
    } while(!succ);
    wait_finish();
}

void sd_init() {
    pin_setup();
    sdhost_setup();
    sdcard_setup();
}

extern struct fat32_meta fat32_meta;

int sd_mount_fat32() {
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
    struct fat32_boot_sector *bs;
    memcpy(buf, bs, sizeof(struct fat32_boot_sector));

    // decide what info need later
    // size in sector
    fat32_meta.sec_size.reserved_sectors = bs->n_reserved_sectors;
    fat32_meta.sec_size.FAT_region = bs->n_FATs * bs->sectors_per_FAT;
    fat32_meta.sec_size.clus_size_byte = bs->bytes_per_logical_sector * bs->logical_sector_per_cluster;
    
    // sector begin
    fat32_meta.sec_beg.reserved_sectors = parti_blk_idx;
    fat32_meta.sec_beg.FAT_region = fat32_meta.sec_beg.reserved_sectors + fat32_meta.sec_size.reserved_sectors;
    fat32_meta.sec_beg.data_region = fat32_meta.sec_beg.FAT_region + fat32_meta.sec_size.FAT_region;
    fat32_meta.sec_beg.root_dir_entry
        = fat32_meta.sec_beg.data_region;


    // 3. Get the root directory cluster number and create it’s root directory object.    
    int ret;
    ret = vfs_mkdir("/boot");               // /boot is TA's spec
    ret = vfs_mount("/boot", "fat32");
    // file_t *boot_dir;
    // vfs_open("/boot", O_CREAT, &boot_dir);
    // struct fat32_inter =  boot_dir->vnode->internal;

    return ret;
}


/*
    uint32_t FAT_blk_count = fat32_meta.sec_size.FAT_region;
    uint32_t FAT_entry_count =  BLOCK_SIZE / sizeof(uint32_t);
    uint32_t FAT_total_entry_count = FAT_blk_count * FAT_entry_count;
    uint32_t *FAT = (uint32_t *)kmalloc(fat32_meta.sec_size.FAT_region * bs->bytes_per_logical_sector);
    // uint32_t FAT[fat32_meta.sec_size.FAT_region * bs->bytes_per_logical_sector / sizeof(uint32_t)];
    // uint32_t FAT[FAT_toal_entry_count];

    // load the whole FAT into local variable FAT
    uint32_t blk_idx = 0;
    while (blk_idx < FAT_blk_count)
    {
        sd_read_block(fat32_meta.sec_beg.FAT_region + blk_idx, FAT + blk_idx * BLOCK_SIZE / sizeof(uint32_t));
        blk_idx++;
        uart_put_int(blk_idx);
    }
    uart_puts("\nOK~~~\n");
    blk_idx = 0;

    // int32_t seen[FAT_total_entry_count];
    int32_t *seen = (int32_t *)kmalloc(FAT_total_entry_count);
    // uint32_t clus_sets[FAT_total_entry_count];
    uint32_t *clus_sets = (uint32_t *)kmalloc(FAT_total_entry_count);
    uint32_t clus_idx = 0;

    // special entries (the first two entries in whole FAT region)
    uint32_t EOCCM = *(FAT + 1); // end of cluster chain marker
    
    // // skip special entry
    // uint32_t entry_idx = 2;                                 
    uint32_t seen_idx = 2;                                 
    seen[0] = seen[1] = -1;                                 // -1 means the extry have seen
 
    uint32_t cur_entry = 2;                                 // skip special entry
    uint32_t next_entry;
    uint32_t count = 0;
    // while (seen_idx < FAT_total_entry_count)
    while (count < FAT_total_entry_count)
    {
        count++;
        // uart_put_int(seen_idx);
        // uart_puts(" ");
        //ret = vfs_open((const char *)dir_entry->name, "fat32");

        if (cur_entry == EOCCM || cur_entry == EOC)
        {
            clus_sets[clus_idx++] = 0;    // use 0 as delimiter between file's clusters
            
            // find unseen clus num
            while (seen[seen_idx] == -1 || *(FAT + seen_idx) == 0)
            {
                seen_idx++;
            }
            next_entry = seen_idx;         
        }
        else if (cur_entry != 0)
        {
            seen[cur_entry] = -1;
            next_entry = *(FAT + cur_entry);
            
            clus_sets[clus_idx++] = cur_entry;
            // clusters_size++;
        }
        cur_entry = next_entry;
    }
*/



//-------------------------------------------------//
//               classic generic MBR               //
//-------------------------------------------------//

/* 
    The MBR is not located in a partition; 
    it is located at a first sector of the device (physical offset 0), 
    preceding the first partition
*/
void read_classical_generic_MBR(char *mbr)
{
    sd_read_block(0, mbr);
}

