#ifndef __VFS_H__
#define __VFS_H__

#include "list.h"

//----------------------------------------------------------//
//                     virtual file system                  //
//----------------------------------------------------------//

// flag
#define O_CREAT (1)

typedef struct vnode {
  //struct mount* mount;
  struct dentry *dentry;  
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  void* internal;
}vnode_t;

typedef struct dentry {
    char name[64];
    struct dentry* parent;          // parent directory
    //struct list_head list;        
    //struct list_head childs;      // our children
    struct list_head d_siblings;    // child of parent list
    struct list_head d_subdirs;     // our children
    struct vnode* vnode;
    //int type;
    //struct mount* mountpoint;
    //struct dentry* mount_origin;
}dentry_t;


// file handle
typedef struct file {
  struct vnode* vnode;
  unsigned long f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
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
//---------- file operation ----------//
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(struct file* file);
int vfs_write(struct file* file, const void* buf, unsigned long len);
int vfs_read(struct file* file, void* buf, unsigned long len);
//---------- vnode operation ----------//
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
int vfs_create(vnode_t* dir_node, vnode_t** v_tar, const char* component_name);


//----------------------------------------------------------//
//                         tmpfs                            //
//----------------------------------------------------------//
int tmpfs_register();
int tmpfs_set_mountup(filesystem_t *tmpfs, struct mount *rootfs);
vnode_t *tmpfs_create_vnode();
dentry_t *tmpfs_create_dentry(vnode_t *v, const char *name, dentry_t *parent);
//---------- file operation ----------//
int tmpfs_write(file_t* file, const void* buf, unsigned long len);
int tmpfs_read(file_t* file, void* buf, unsigned long len);
int tmpfs_open(vnode_t* file_node, file_t** target);
int tmpfs_close(file_t* file);
//---------- vnode operation ----------//
int tmpfs_lookup(vnode_t* dir_node, vnode_t** target, const char* component_name);
int tmpfs_create(vnode_t* dir_node, vnode_t** target, const char* component_name);
int tmpfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name);


#endif