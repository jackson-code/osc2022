#ifndef _INIT_RAM_FS_H_
#define _INIT_RAM_FS_H_

#include "vfs.h"

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

#endif