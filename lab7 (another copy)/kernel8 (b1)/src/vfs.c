#include "vfs.h"
#include "mm.h"
#include "my_string.h"
#include "uart.h"

char *get_component_name(const char *pathname) {
    int len = str_len(pathname);
    char *res;
    int idx = len - 1;  // begin from last char
    while (*(pathname + idx) != '/' || idx > 0)
    {
        *res++ = *(pathname + idx);
        idx++;
    }
    *res = '\0';
    return res;
}

//----------------------------------------------------------//
//                     virtual file system                  //
//----------------------------------------------------------//
/*
    register and mountup tmpfs as root file system.
*/
void rootfs_init() {
	uart_puts("rootfs_init begin\n");

    filesystem_t *tmpfs = (filesystem_t *)kmalloc(sizeof(filesystem_t *));
    tmpfs = (filesystem_t *)kmalloc(sizeof(filesystem_t));
    tmpfs->name = "tmpfs";
    tmpfs->setup_mount = tmpfs_set_mountup;

    // register tmpfs
    register_filesystem(tmpfs);

    // mount up tmpfs
    rootfs = (struct mount *)kmalloc(sizeof(struct mount *));
    tmpfs->setup_mount(tmpfs, rootfs);

	uart_puts("rootfs_init finish\n");
}


int register_filesystem(struct filesystem* fs) {
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    if (!str_cmp("tmpfs", fs->name))
    {
        return tmpfs_register();
    }
    return -1;
}

//---------- file operation ----------//

int vfs_open(const char* pathname, int flags, file_t** file_tar) {
    vnode_t *v_tar;
    // 1. Lookup pathname
    int result = vfs_lookup(pathname, &v_tar);
    // 2. Create a new file handle for this vnode if found.
    char *component_name = get_component_name(pathname);
    if (result == 0)
    {
        vfs_create(v_tar, &v_tar, component_name);
    } else
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    {
        if (flags == O_CREAT)
        {
            vnode_t* new_node;
            vfs_create(v_tar, &new_node, component_name);
        }
    }

    // 4. Return error code if fails        

    return 0;
}

int vfs_close(struct file* file) {
  // 1. release the file handle
  // 2. Return error code if fails
  return 0;
}

int vfs_write(struct file* file, const void* buf, unsigned long len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  return 0;
}

int vfs_read(struct file* file, void* buf, unsigned long len) {
  // 1. read min(len, readable size) byte to buf from the opened file.
  // 2. block if nothing to read for FIFO type
  // 2. return read size or error code if an error occurs.
  return 0;
}
//-------------------------------------//


//---------- vnode operation ----------//
int vfs_mkdir(const char* pathname) {
    return 0;
}
int vfs_lookup(const char* pathname, vnode_t** v_tar) {
    return rootfs->root->v_ops->lookup(rootfs->root, v_tar, pathname);
}
int vfs_create(vnode_t* dir_node, vnode_t** v_tar, const char* component_name) {
    return rootfs->root->v_ops->create(dir_node, v_tar, component_name);
}

//----------------------------------------------------------//
//                         tmpfs                            //
//----------------------------------------------------------//
file_op_t *tmpfs_f_op;
vnode_op_t *tmpfs_v_op;

int tmpfs_register() {
    tmpfs_f_op = (file_op_t *)kmalloc(sizeof(file_op_t));
    tmpfs_f_op->open = tmpfs_open;
    tmpfs_f_op->close = tmpfs_close;
    tmpfs_f_op->read = tmpfs_read;
    tmpfs_f_op->write = tmpfs_write;

    tmpfs_v_op = (vnode_op_t *)kmalloc(sizeof(vnode_op_t));
    tmpfs_v_op->create = tmpfs_create;
    tmpfs_v_op->lookup = tmpfs_lookup;
    tmpfs_v_op->mkdir = tmpfs_mkdir;

    return 0;
}

// create root vnode & dentry
int tmpfs_set_mountup(filesystem_t *tmpfs, struct mount *rootfs) {
    rootfs->fs = tmpfs;
    rootfs->root = tmpfs_create_vnode();
    tmpfs_create_dentry(rootfs->root, "/", 0);
    return 0;
}

vnode_t *tmpfs_create_vnode() {
    vnode_t *v = (vnode_t *)kmalloc(sizeof(vnode_t));
    v->dentry = 0;
    v->f_ops = tmpfs_f_op;
    v->v_ops = tmpfs_v_op;
    v->internal = (void *)0;
    //v->mount = 
    return v;
}

/*
    call tmpfs_create_vnode() first, then call this one
*/
dentry_t *tmpfs_create_dentry(vnode_t *v, const char *name, dentry_t *parent) {
    dentry_t *d = (dentry_t *)kmalloc(sizeof(dentry_t));
    str_cpy(d->name, name);
    d->vnode =  v;
    v->dentry = d;
    list_init(&d->d_subdirs);
    list_init(&d->d_siblings);
    d->parent = parent;
    if (parent != 0)
    {
        list_push(&parent->d_subdirs, (list_head *)d);
    }
    
    return d;
}



//---------- file operation ----------//
int tmpfs_write(file_t* file, const void* buf, unsigned long len) {
  return 0;

}

int tmpfs_read(file_t* file, void* buf, unsigned long len) {

  return 0;
}

int tmpfs_open(vnode_t* file_node, file_t** target) {

  return 0;
}

int tmpfs_close(file_t* file) {

  return 0;
}
//-------------------------------------//


//---------- vnode operation ----------//
int tmpfs_taversal(vnode_t *cur, const char* pathname, vnode_t** v_tar) {
    char *component;
    while (*pathname != '/' || *pathname != '\0')
    {
        *component++ = *pathname++;
    }
    component = '\0';

    v_tar = &cur;

    if (*pathname == '\0')  // last component in path
    {
        return 0;
    }
    
    // finding in child
    for (list_head *subdir = cur->dentry->d_subdirs.next; subdir != &cur->dentry->d_subdirs; subdir = subdir->next)
    {
        struct dentry* d_child = list_entry(subdir, struct dentry, d_siblings);
        if (!str_cmp(d_child->name, component))
        {
            return tmpfs_taversal(d_child->vnode, ++pathname, v_tar);
        }
        else    // can't find in child
        {
            return -1;
        }
    }
    return -1;
}
int tmpfs_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* pathname) {
    if (pathname[0] == '/')     // absulute path
    {
        return tmpfs_taversal(rootfs->root, ++pathname, v_tar);
    }
    else    // relative path
    {
        return -1;
    }    
}

int tmpfs_create(vnode_t* dir_node, vnode_t** target, const char* component_name) {
    if (target == 0)
    {
        vnode_t *new = tmpfs_create_vnode();
        target = &new;
        tmpfs_create_vnode(target, component_name, dir_node);
    }
    else
    {
        file_t* fd = (file_t *)kmalloc(sizeof(file_t));
        fd->vnode = *target;
        fd->f_ops = (*target)->f_ops;
        fd->f_pos = 0;
    }
    return 0; 
}

int tmpfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name) {

  return 0;
}
//-------------------------------------//
