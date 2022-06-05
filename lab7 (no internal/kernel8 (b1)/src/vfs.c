#include "vfs.h"
#include "mm.h"
#include "my_string.h"
#include "uart.h"

char *get_last_component_name(const char *pathname) {
    int len = str_len(pathname);
    char *res;
    int idx = len - 1;  // begin from last char
    while (*(pathname + idx) != '/' && idx > 0)
    {
        *res++ = *(pathname + idx);
        idx--;
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
    vnode_t *dir_node;
    // 1. Lookup pathname
    int ret = vfs_lookup(pathname, &dir_node);

    // get file name
	int comp_count = str_token_count(pathname, '/');
	char *comp_names[comp_count];
    int max_name_len = 64;
	for (int j = 0; j < comp_count; j++) {
		comp_names[j] = (char *)kmalloc(max_name_len);
	}
    str_token(pathname, comp_names, '/');
    char *file_name = comp_names[comp_count - 1];

    // 2. Create a new file handle for this vnode if found.
    if (ret == 0)
    {
        vfs_create(dir_node, &dir_node, file_name);
    } else
    // 3. Create a new file(and vnode, dentry) if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    {
        if (flags == O_CREAT)
        {
            vnode_t* new_node = 0;
            vfs_create(dir_node, &new_node, file_name);
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
    // get component name
	int comp_count = str_token_count(pathname, '/');
	char *comp_names[comp_count];
    int max_name_len = 64;
	for (int j = 0; j < comp_count; j++) {
		comp_names[j] = (char *)kmalloc(max_name_len);
	}
    str_token(pathname, comp_names, '/');
    
    if (pathname[0] == '/')     // absulute path
    {
        vnode_t *vnode_itr = rootfs->root;
        for (int i = 0; i < comp_count; i++) {
            vnode_t *next_vnode;
            int ret = vnode_itr->v_ops->lookup(vnode_itr, &next_vnode, comp_names[i]);
            if(ret != 0) {
                *v_tar = vnode_itr;     // return directory node
                return ret;             // can't find
            }
            vnode_itr = next_vnode;
        }
        *v_tar = vnode_itr;
        return 0;
    }
    else    // relative path
    {
        return 1111;
    }    
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
    rootfs->root = tmpfs_create_vnode("/", 0, DIRECTORY);
    return 0;
}

vnode_t *tmpfs_create_vnode(const char *name, vnode_t *parent, enum vnode_type type) {
    vnode_t *v = (vnode_t *)kmalloc(sizeof(vnode_t));
    v->mount = rootfs;
    v->f_ops = tmpfs_f_op;
    v->v_ops = tmpfs_v_op;
    v->internal = 0;

    v->file = 0;
    v->parent = parent;
    str_cpy(v->name, name);
    v->type = type;
    list_init(&v->siblings);
    list_init(&v->subdirs);
    if (parent != 0)
    {
        list_push(&parent->subdirs, (list_head *)v);
    }
    return v;
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
int tmpfs_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* component_name) {
    // finding file's dentry in child
    for (list_head *subdir = dir_node->subdirs.next; subdir != &dir_node->subdirs; subdir = subdir->next)
    {
        vnode_t *child = list_entry(subdir, struct vnode, subdirs);
        if (!str_cmp(child->name, component_name))
        {
            *v_tar = child;
            return 0;
        }
    }
    return -1;
}

int tmpfs_create(vnode_t* dir_node, vnode_t** target, const char* file_name) {
    if (*target == 0)    // the file's vnode not existing
    {
        vnode_t *new = tmpfs_create_vnode(file_name, dir_node, REGULAR_FILE);
        //target = &new;
        *target = new;
    }

    file_t* fd = (file_t *)kmalloc(sizeof(file_t));
    fd->vnode = *target;
    fd->f_ops = (*target)->f_ops;
    fd->f_pos = 0;

    (*target)->file = fd;

    return 0; 
}

int tmpfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name) {

  return 0;
}
//-------------------------------------//
