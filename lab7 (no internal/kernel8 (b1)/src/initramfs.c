#include "initramfs.h"
#include "mm.h"
#include "uart.h"
#include "my_string.h"

//----------------------------------------------------------//
//                         initramfs                        //
//----------------------------------------------------------//
file_op_t *initramfs_f_op;
vnode_op_t *initramfs_v_op;

int initramfs_register() {
    initramfs_f_op = (file_op_t *)kmalloc(sizeof(file_op_t));
    initramfs_f_op->open = initramfs_open;
    initramfs_f_op->close = initramfs_close;
    initramfs_f_op->read = initramfs_read;
    initramfs_f_op->write = initramfs_write;

    initramfs_v_op = (vnode_op_t *)kmalloc(sizeof(vnode_op_t));
    initramfs_v_op->create = initramfs_create;
    initramfs_v_op->lookup = initramfs_lookup;
    initramfs_v_op->mkdir = initramfs_mkdir;

    return 0;
}

vnode_t *initramfs_create_vnode(const char *name, vnode_t *parent, enum vnode_type type) {
    vnode_t *v = (vnode_t *)kmalloc(sizeof(vnode_t));
    if (parent != 0)
    {
        v->mount = parent->mount;
    }
    else
    {
        v->mount = rootfs;
    }
    v->f_ops = initramfs_f_op;
    v->v_ops = initramfs_v_op;
    v->internal = 0;

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

    // debug message
    uart_puts("initramfs_create_vnode():\n\tvnode at 0x");
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

int initramfs_create_file(vnode_t* file_node, file_t** target) {
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

        // won't into the if, if the file wasn't opened first time
        if (file_node->internal == 0)   // first time to establish the file
        {
            // alloc space for file's content
            inter_tmpfs_t *inter = (inter_tmpfs_t *)kmalloc(sizeof(inter_tmpfs_t));
            inter->file_content = (char *)kmalloc(TMPFS_MAX_FILE_SIZE);
            file_node->internal = inter;
        }

        return 0;
    }
    else
    {
        *target = file_node->file;
        //(*target)->status = FILE_EXIST;
        uart_puts("\tWARNING initramfs_create_file(): file existing\n");
        return 0;
    }
}

//---------- file operation ----------//
int initramfs_write(file_t* file, const void* buf, unsigned long len) {
    return -1;
}

int initramfs_read(file_t* file, void* buf, unsigned long len) {
    char *src = ((inter_tmpfs_t *)(file->vnode->internal))->file_content;
    char *des = (char *)buf;
    src += file->f_pos;
    char *src_origin = src;

    while((len--) > 0 && *src != EOF){
		*des++ = *src++;
	}

    unsigned long read_size = src - src_origin;
    file->f_pos += read_size;

    return read_size;
}

/*
    open the vnode, create file to the vnode
*/
int initramfs_open(vnode_t* file_node, file_t** target) {
    if (file_node->type == REGULAR_FILE)
    {
        return initramfs_create_file(file_node, target); 
    }
    else
    {
        uart_puts("\tERROR in initramfs_open(): can't open directory vnode\n");
        return -1;
    }
}

int initramfs_close(file_t* file) {
    file->status = FILE_NOT_EXIST;
    file->vnode->file = 0;
    //kfree(((inter_tmpfs_t *)file->vnode->internal)->file_content);
    //kfree(file->vnode);
    kfree(file);
    return 0;
}
//-------------------------------------//


//---------- vnode operation ----------//
int initramfs_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* component_name) {
    // finding vnode in dir_node's child
    for (list_head *sibling = dir_node->subdirs.next; sibling != &dir_node->subdirs; sibling = sibling->next)
    {
        vnode_t *child = list_entry(sibling, struct vnode, siblings);
        if (!str_cmp(child->name, component_name))
        {
            *v_tar = child;
            return 0;
        }
    }
    return -1;
}

int initramfs_create(vnode_t* dir_node, vnode_t** file_node, const char* file_name) {
    return -1; 
}

int initramfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name) {
    return -1;
}
//-------------------------------------//