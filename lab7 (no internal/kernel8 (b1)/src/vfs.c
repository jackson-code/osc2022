#include "vfs.h"
#include "mm.h"
#include "my_string.h"
#include "uart.h"
#include "utils.h"
#include "process.h"
#include "scheduler.h"
#include "mailbox.h"

file_op_t *initramfs_f_op;
vnode_op_t *initramfs_v_op;

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
    if (!str_cmp("initramfs", fs->name))
    {
        return initramfs_register();
    }
    return -1;
}

int vfs_traversal(const char* target, vnode_t *dir_node, vnode_t **v_tar) {
    // search in the same level
    int ret = dir_node->v_ops->lookup(dir_node, v_tar, target);
    if(ret == 0) {  // find!
        return ret;             
    }
    else
    {   // search in the next level
        for (list_head *sibling = dir_node->subdirs.next; sibling != &dir_node->subdirs; sibling = sibling->next)
        {
            vnode_t *child = list_entry(sibling, struct vnode, siblings);
            ret = vfs_traversal(target, child, v_tar);
            if(ret == 0) {  // find!
                return ret;             
            }
        }
        return -1;
    }
}

int vfs_mount(const char* target, const char* filesystem) {
	uart_puts("vfs_mount begin\n");

    filesystem_t *fs = (filesystem_t *)kmalloc(sizeof(filesystem_t *));
    fs = (filesystem_t *)kmalloc(sizeof(filesystem_t));
    fs->name = filesystem;
    // fs->set_mountup

    // register fs
    register_filesystem(fs);

    // find target vnode
    int ret;
    vnode_t *v_tar; 
    if (target[0] == '/' && target[1] == '\0')
    {
        v_tar = rootfs->root;   // mount fs on root
    }
    else
    {
        if (target[0] == '/' && target[1] != '\0')
            target++;           // remove '/'
        ret = vfs_traversal(target, rootfs->root, &v_tar);
        if (ret != 0)
            uart_puts("\tERROR in vfs_traversal(): can't find target vnode\n");
    }
    
    // mount up fs
    struct mount *mount_fs = (struct mount *)kmalloc(sizeof(struct mount));
    mount_fs->fs = fs;
    mount_fs->root = v_tar;
    v_tar->mount = mount_fs;

    // register method
    // register tmpfs's method by default
    if (!str_cmp("initramfs", filesystem))
    {
        v_tar->f_ops = initramfs_f_op;
        v_tar->v_ops = initramfs_v_op;
    }
    

	uart_puts("vfs_mount finish\n");
    return ret;
}

int vfs_chdir(const char *pathname) {
    vnode_t *v_tar;
    int ret = vfs_lookup(pathname, &v_tar);
    if (ret == 0 && v_tar->type == DIRECTORY)
    {
        Task *cur_task = sche_running_proc(&sche_proc);
        cur_task->dir_node = v_tar;
        return 0;
    }
    else if (ret != 0) {
        uart_puts("ERROR in vfs_chdir():\tcan't find the vnode\n");
        return -1;
    }
    else if (ret == 0 && v_tar->type == REGULAR_FILE) {
        uart_puts("ERROR in vfs_chdir():\tfind the vnode, but it's regular file vnode\n");
        return -1;
    }
    else {
        uart_puts("ERROR in vfs_chdir():\tunknown error\n");
        return -1;
    }
}

int vfs_mknod(const char *pathname, enum dev_type dev_type){
    if (dev_type == UART)
    {
        vnode_t *dev;
        
        // get dev vnode 
        vfs_lookup(pathname, &dev);

        sf_uart_register();
        return sf_uart_create(dev);
    }
    else if (dev_type == FRAME_BUFFER)
    {
        mailbox_init_framebuffer();

        vnode_t *dev;
        
        // get dev vnode 
        vfs_lookup(pathname, &dev);

        sf_fb_register();
        return sf_fb_create(dev);

        return -1;
    }
    else
    {
        uart_puts("ERROR in vfs_mknod():\t unsupporting device\n");
        return -1;
    }
}

//---------- file operation ----------//
int vfs_open(const char* pathname, int flags, file_t** file_tar) {
    // get file name (TODO: free comp_names)
	int comp_count = str_token_count(pathname, '/');
	char *comp_names[comp_count];
    int max_name_len = 64;
	for (int j = 0; j < comp_count; j++) {
		comp_names[j] = (char *)kmalloc(max_name_len);
	}
    str_token(pathname, comp_names, '/');
    char *file_name = comp_names[comp_count - 1];

    vnode_t *node;          // will be directory/file vnode
    // 1. Lookup pathname
    int ret = vfs_lookup(pathname, &node);

    // special file
    if (ret == 0 && node->type == SPECIAL_FILE)
    {
        *file_tar = node->file;
        return 0;
    }     

    // 2. Create a new file handle for this vnode if found.
    if (ret == 0 && node->type == REGULAR_FILE)
    {
        //str_cpy(node->name, file_name);
        return node->f_ops->open(node, file_tar);
    } 
    else if (!str_cmp(node->name, "initramfs"))
    {
        return node->f_ops->open(node, file_tar);
    } 
    else
    // 3. Create a new file(and vnode) if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    {
        if (flags == O_CREAT && node->type == DIRECTORY)
        {
            // the node is directory node, be the parent of file vnode
            vnode_t *file_node = (vnode_t *)kmalloc(sizeof(vnode_t));
            ret = vfs_create(node, &file_node, file_name);  // create file & vnode
            *file_tar = file_node->file;
        }
    }

    // 4. Return error code if fails   
    return ret;     
}

int vfs_close(struct file* file) {
    // 1. release the file handle
    // 2. Return error code if fails
    return file->vnode->f_ops->close(file);
}

int vfs_write(struct file* file, const void* buf, unsigned long len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    if (file->status == FILE_EXIST)
    {
        return file->f_ops->write(file, buf, len);
    }
    uart_puts("\tERROR in vfs_write(): file not existing\n");
    return -1;
}

int vfs_read(struct file* file, void* buf, unsigned long len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    if (file->status == FILE_EXIST)
    {
        return file->f_ops->read(file, buf, len);
    }
    uart_puts("\tERROR in vfs_read(): file not existing\n");
    return -1;
}
//-------------------------------------//


//---------- vnode operation ----------//
int vfs_mkdir(const char* pathname) {
    // get file name (TODO: free comp_names)
	int comp_count = str_token_count(pathname, '/');
	char *comp_names[comp_count];
    int max_name_len = 64;
	for (int j = 0; j < comp_count; j++) {
		comp_names[j] = (char *)kmalloc(max_name_len);
	}
    str_token(pathname, comp_names, '/');

    vnode_t *dir_node;          // will be parent dir vnode
    // 1. Lookup pathname, get the parent dir vnode
    int ret = vfs_lookup(pathname, &dir_node);

    // 2. Create a new dir vnode
    if (ret != 0 && dir_node->type == DIRECTORY)
    {
        vnode_t *target = (vnode_t *)kmalloc(sizeof(vnode_t));
        return dir_node->v_ops->mkdir(dir_node, &target, comp_names[comp_count - 1]);
    }
    else
    {
        uart_puts("WARNING in vfs_mkdir():\tdirectory vnode has been existing\n");
        return -1;
    }
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
    
    vnode_t *vnode_itr;
    int i;
    if (pathname[0] == '/')     // absulute path
    {
        vnode_itr = rootfs->root;
        i = 0;
    }
    else                        // relative path
    {
        Task *cur_task = sche_running_proc(&sche_proc);
        i = 1;      // skip '.' or '..' in pathname  

        if (pathname[0] == '\0')                            // return root vnode                          
        {
            *v_tar = cur_task->dir_node->mount->root;
            return 0;
        } 
        else if (pathname[0] == '.'&& pathname[1] != '.')   // return current dir
        {
            vnode_itr = cur_task->dir_node;
        }
        else if (pathname[0] == '.' && pathname[1] == '.')  // return upper dir
        {
            vnode_itr = cur_task->dir_node->parent;
        }
        else
        {
            uart_puts("\tERROR in vfs_lookup(): error format of pathname\n");
            return -1;
        }
    }

    for (; i < comp_count; i++) {
        vnode_t *next_vnode;
        int ret = vnode_itr->v_ops->lookup(vnode_itr, &next_vnode, comp_names[i]);
        if(ret != 0) {              // can't find
            *v_tar = vnode_itr;     // return directory node
            return ret;             
        }
        vnode_itr = next_vnode;
    }
    *v_tar = vnode_itr;
    return 0;
}

int vfs_create(vnode_t* dir_node, vnode_t** file_node, const char* component_name) {
    return rootfs->root->v_ops->create(dir_node, file_node, component_name);
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
    if (parent != 0)
    {
        v->mount = parent->mount;
    }
    else
    {
        v->mount = rootfs;
    }
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
        // list_push(&parent->subdirs, &v->subdirs);
        list_push(&parent->subdirs, &v->siblings);
    }
    uart_puts("tmpfs_create_vnode():\n\tvnode at 0x");
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

int tmpfs_create_file(vnode_t* file_node, file_t** target) {
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
        uart_puts("\tWARNING tmpfs_create_file(): file existing\n");
        return 0;
    }
}

//---------- file operation ----------//
int tmpfs_write(file_t* file, const void* buf, unsigned long len) {
    char *src = (char *)buf;
    char *des = ((inter_tmpfs_t *)(file->vnode->internal))->file_content;
    des += file->f_pos;
    char *des_origin = des;

    while((len--) > 0 && *src != '\0'){
		*des++ = *src++;
	}
    *des = EOF;

    unsigned long written_size = des - des_origin;
    file->f_pos += written_size;
    file->size += written_size;

    return written_size;
}

int tmpfs_read(file_t* file, void* buf, unsigned long len) {
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
int tmpfs_open(vnode_t* file_node, file_t** target) {
    if (file_node->type == REGULAR_FILE)
    {
        return tmpfs_create_file(file_node, target); 
    }
    else
    {
        uart_puts("\tERROR in tmpfs_open(): can't open directory vnode\n");
        return -1;
    }
}

int tmpfs_close(file_t* file) {
    file->status = FILE_NOT_EXIST;
    file->vnode->file = 0;
    //kfree(((inter_tmpfs_t *)file->vnode->internal)->file_content);
    //kfree(file->vnode);
    kfree(file);
    return 0;
}
//-------------------------------------//


//---------- vnode operation ----------//
int tmpfs_lookup(vnode_t* dir_node, vnode_t** v_tar, const char* component_name) {
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

/*
    crate vnode & file
*/
int tmpfs_create(vnode_t* dir_node, vnode_t** file_node, const char* file_name) {
    //if ((*file_node)->name == "\0")    // the file's vnode not existing
    if (!str_cmp("\0", (*file_node)->name))    // the file's vnode not existing
    {
        // create file's vnode below dir_node 
        *file_node = tmpfs_create_vnode(file_name, dir_node, REGULAR_FILE);

        file_t *file = (file_t *)kmalloc(sizeof(file_t));
        return tmpfs_create_file(*file_node, &file); 
    }
    else
    {
        uart_puts("\tERROR in tmpfs_create(): file existing");
        return -1; 
    }
}

int tmpfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name) {
    *target = tmpfs_create_vnode(component_name, dir_node, DIRECTORY);
    return 0;
}
//-------------------------------------//



//----------------------------------------------------------//
//                         initramfs                        //
//----------------------------------------------------------//
void initramfs_init() {
    vfs_mkdir("/initramfs");
	vfs_mount("/initramfs", "initramfs");    
 }

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

// vnode_t *initramfs_create_vnode(const char *name, vnode_t *parent, enum vnode_type type) {
//     vnode_t *v = (vnode_t *)kmalloc(sizeof(vnode_t));
//     if (parent != 0)
//     {
//         v->mount = parent->mount;
//     }
//     else
//     {
//         v->mount = rootfs;
//     }
//     v->f_ops = initramfs_f_op;
//     v->v_ops = initramfs_v_op;
//     v->internal = 0;

//     v->file = 0;
//     v->parent = parent;
//     str_cpy(v->name, name);
//     v->type = type;
//     list_init(&v->siblings);
//     list_init(&v->subdirs);
//     if (parent != 0)
//     {
//         list_push(&parent->subdirs, &v->siblings);
//     }

//     // debug message
//     uart_puts("initramfs_create_vnode():\n\tvnode at 0x");
//     uart_put_hex((unsigned long)v);
//     if (parent != 0) {
//         uart_puts("\tparent->subdirs at 0x");
//         uart_put_hex((unsigned long)&parent->subdirs);
//     }
//     uart_puts("\tv->siblings at 0x");
//     uart_put_hex((unsigned long)&v->siblings);
//     uart_puts(",\tsubdirs at 0x");
//     uart_put_hex((unsigned long)&v->subdirs);
//     uart_puts("\n");
//     return v;
// }

int initramfs_create_file(vnode_t* file_node, file_t** target) {
    uart_puts("\t***** initramfs_create_file *****\n");

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
    uart_puts("\t***** initramfs_write *****\n");
    return -1;
}

int initramfs_read(file_t* file, void* buf, unsigned long len) {
    uart_puts("\t***** initramfs_read *****\n");

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
    uart_puts("\t***** initramfs_open *****\n");
    
    if (file_node->type == DIRECTORY)   // iniramfs have no create method, so no regular file
    {
        return initramfs_create_file(file_node, target); 
    }
    else
    {
        uart_puts("\tERROR in initramfs_open(): can't open regular file vnode\n");
        return -1;
    }
}

int initramfs_close(file_t* file) {
    uart_puts("\t***** initramfs_close *****\n");
    
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
    uart_puts("\t***** initramfs_lookup *****\n");

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
    uart_puts("\t***** initramfs_create *****\n");
    return -1; 
}

int initramfs_mkdir(vnode_t* dir_node, vnode_t** target, const char* component_name) {
    uart_puts("\t***** initramfs_mkdir *****\n");
    return -1;
}
//-------------------------------------//


//----------------------------------------------------------//
//                    uart special file                     //
//----------------------------------------------------------//
file_op_t *uart_f_op;
vnode_op_t *uart_v_op;
/*
    create uart's vnode & file below dev vnode
*/
int sf_uart_create(vnode_t *dev) {
    // create uart's vnode
    vnode_t *u_node = (vnode_t *)kmalloc(sizeof(vnode_t));
    u_node->mount = dev->mount;
    u_node->f_ops = uart_f_op;
    u_node->v_ops = uart_v_op;
    u_node->internal = 0;

    u_node->file = 0;
    u_node->parent = dev;
    str_cpy(u_node->name, "uart");
    u_node->type = SPECIAL_FILE;
    list_init(&u_node->siblings);
    list_init(&u_node->subdirs);
    list_push(&dev->subdirs, &u_node->siblings);

    // create uart's file
    file_t *u_file = (file_t *)kmalloc(sizeof(file_t));
    u_file->vnode = u_node;
    u_file->f_ops = u_node->f_ops;
    u_file->f_pos = 0;
    u_file->size = 0;
    u_file->status = FILE_EXIST;

    u_node->file = u_file;

    return 0;
}

int sf_uart_register() {
    uart_f_op = (file_op_t *)kmalloc(sizeof(file_op_t));
    uart_f_op->open = 0;
    uart_f_op->close = 0;
    uart_f_op->read = sf_uart_read;
    uart_f_op->write = sf_uart_write;

    uart_v_op = (vnode_op_t *)kmalloc(sizeof(vnode_op_t));
    uart_v_op->create = 0;
    uart_v_op->lookup = 0;
    uart_v_op->mkdir = 0;

    return 0;
}

int sf_uart_write(file_t* file, const void* buf, unsigned long len) {
    char *src = (char *)buf;
    while (*src != '\0' && len > 0)
    {
        uart_send(*src);
        src++;
        len--;
    }
    return src - (char *)buf;
}

int sf_uart_read(file_t* file, void* buf, unsigned long len) {
    char *des = (char *)buf;
    while (len > 0)
    {
        *des = uart_getc_raw();
        des++;
        len--;
    }
    return des - (char *)buf;
}


//----------------------------------------------------------//
//                framebuffer special file                  //
//----------------------------------------------------------//
file_op_t *fb_f_op;
vnode_op_t *fb_v_op;
/*
    create uart's vnode & file below dev vnode
*/
int sf_fb_create(vnode_t *dev) {
    // create framebuffer's vnode
    vnode_t *fb_node = (vnode_t *)kmalloc(sizeof(vnode_t));
    fb_node->mount = dev->mount;
    fb_node->f_ops = fb_f_op;
    fb_node->v_ops = fb_v_op;
    fb_node->internal = 0;

    fb_node->file = 0;
    fb_node->parent = dev;
    str_cpy(fb_node->name, "framebuffer");
    fb_node->type = SPECIAL_FILE;
    list_init(&fb_node->siblings);
    list_init(&fb_node->subdirs);
    list_push(&dev->subdirs, &fb_node->siblings);

    // create framebuffer's file
    file_t *fb_file = (file_t *)kmalloc(sizeof(file_t));
    fb_file->vnode = fb_node;
    fb_file->f_ops = fb_node->f_ops;
    fb_file->f_pos = 0;
    fb_file->size = 0;
    fb_file->status = FILE_EXIST;

    fb_node->file = fb_file;

    return 0;
}

int sf_fb_register() {
    fb_f_op = (file_op_t *)kmalloc(sizeof(file_op_t));
    fb_f_op->open = sf_fb_open;
    fb_f_op->close = sf_fb_close;
    fb_f_op->read = sf_fb_read;
    fb_f_op->write = sf_fb_write;

    fb_v_op = (vnode_op_t *)kmalloc(sizeof(vnode_op_t));
    fb_v_op->create = 0;
    fb_v_op->lookup = 0;
    fb_v_op->mkdir = 0;

    return 0;
}

int sf_fb_open(vnode_t* file_node, file_t** target) {
    if (file_node->type == SPECIAL_FILE)
    {
        *target = file_node->file;
        if ((*target)->status == FILE_NOT_EXIST)
        {
            (*target)->status = FILE_EXIST;
        }
        return 0;
    }
    else
    {
        uart_puts("ERROR in sf_fb_open():\tfile_node is not a SPECIAL FILE\n");
        return -1;
    }
}

int sf_fb_close(file_t* file) {
    file->status = FILE_NOT_EXIST;
    file->vnode->file = 0;
    kfree(file);
    return 0;
}

int sf_fb_write(file_t* file, const void* buf, unsigned long len) {
    char *src = (char *)buf;
    while (*src != '\0' && len > 0)
    {
        uart_send(*src);
        src++;
        len--;
    }
    return src - (char *)buf;
}

// write only
int sf_fb_read(file_t* file, void* buf, unsigned long len) {
    return -1;
}