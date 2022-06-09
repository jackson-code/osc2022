#ifndef __VFS_H__
#define __VFS_H__

#include "list.h"

#define EOF   (0xff)

#define VFS_MAX_PATHNAME          (256)
#define FD_RESERVED               (3) // stdin (fd 0), stdout (fd 1), and stderr (fd 2)


//----------------------------------------------------------//
//                     virtual file system                  //
//----------------------------------------------------------//

// flag
#define O_CREAT (00000100)

enum dev_type {
	UART = 1,
	FRAME_BUFFER = 2,
};

enum vnode_type {
    REGULAR_FILE = 1,
    DIRECTORY = 2,
	SPECIAL_FILE = 3,
};

typedef struct vnode {
    struct mount* mount;
    struct vnode_operations* v_ops;
    struct file_operations* f_ops;
    void* internal;

    char name[64];
    struct file *file;
    struct vnode *parent;
    enum vnode_type type;
    struct list_head siblings;    // child of parent list
    struct list_head subdirs;     // our children
}vnode_t;

enum file_status {
    FILE_EXIST = 1,
    FILE_NOT_EXIST = 2,
};

// file handle
typedef struct file {
    struct vnode *vnode;
    unsigned long f_pos;  // RW position of this file handle
    struct file_operations* f_ops;
    int flags;
    unsigned long size;
    enum file_status status;
}file_t;

struct mount {
    struct vnode* root;
    struct filesystem* fs;
};

typedef struct filesystem {
    const char* name;
    int (*setup_mount)(struct filesystem* fs, struct mount* mount);
}filesystem_t;

typedef struct file_operations {
    int (*write)(struct file* file, const void* buf, unsigned long len);
    int (*read)(struct file* file, void* buf, unsigned long len);
    int (*open)(struct vnode* file_node, struct file** target);
    int (*close)(struct file* file);
    //long lseek64(struct file* file, long offset, int whence);
}file_op_t;

typedef struct vnode_operations {
    int (*lookup)(struct vnode* dir_node, struct vnode** target,
                  const char* component_name);
    int (*create)(struct vnode* dir_node, struct vnode** target,
                  const char* component_name);
    int (*mkdir)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
}vnode_op_t;

struct mount* rootfs;

void rootfs_init();
int register_filesystem(struct filesystem* fs);
int vfs_mount(const char* target, const char* filesystem);
int vfs_chdir(const char *pathname);
int vfs_mknod(const char *pathname, enum dev_type dev);
//---------- file operation ----------//
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, unsigned long len);
int vfs_read(struct file* file, void* buf, unsigned long len);
//---------- vnode operation ----------//
int vfs_mkdir(const char* pathname);
int vfs_lookup(const char* pathname, struct vnode** target);
int vfs_create(vnode_t* dir_node, vnode_t** v_tar, const char* component_name);


//----------------------------------------------------------//
//                         tmpfs                            //
//----------------------------------------------------------//
#define TMPFS_MAX_FILE_SIZE         (4096)  // 4KB
#define TMPFS_MAX_COMPONENT_NAME    (15)
#define TMPFS_MAX_COMPONENT_COUNT   (256)

typedef struct internal_tmpfs {
   char *file_content;
    // char buf[TMPFS_MAX_FILE_SIZE];
}inter_tmpfs_t;


int tmpfs_register();
int tmpfs_set_mountup(filesystem_t *tmpfs, struct mount *rootfs);
vnode_t *tmpfs_create_vnode(const char *name, vnode_t *parent, enum vnode_type type);
//---------- file operation ----------//
int tmpfs_write(file_t* file, const void* buf, unsigned long len);
int tmpfs_read(file_t* file, void* buf, unsigned long len);
int tmpfs_open(vnode_t* file_node, file_t** target);
int tmpfs_close(file_t* file);
//---------- vnode operation ----------//
int tmpfs_lookup(vnode_t* dir_node, vnode_t** target, const char* component_name);
int tmpfs_create(vnode_t* dir_node, vnode_t** target, const char* component_name);
int tmpfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name);


//----------------------------------------------------------//
//                         initramfs                        //
//----------------------------------------------------------//
void initramfs_init();
int initramfs_register();
vnode_t *initramfs_create_vnode(const char *name, vnode_t *parent, enum vnode_type type);
int initramfs_create_file(vnode_t* file_node, file_t** target);
//---------- file operation ----------//
int initramfs_write(file_t* file, const void* buf, unsigned long len);
int initramfs_read(file_t* file, void* buf, unsigned long len);
int initramfs_open(vnode_t* file_node, file_t** target);
int initramfs_close(file_t* file);
//---------- vnode operation ----------//
int initramfs_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* component_name);
int initramfs_create(vnode_t* dir_node, vnode_t** file_node, const char* file_name);
int initramfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name);
//-------------------------------------//

//----------------------------------------------------------//
//                    uart special file                     //
//----------------------------------------------------------//
int sf_uart_create(vnode_t *dev);
int sf_uart_register();
// file operation
int sf_uart_write(file_t* file, const void* buf, unsigned long len);
int sf_uart_read(file_t* file, void* buf, unsigned long len);

#endif