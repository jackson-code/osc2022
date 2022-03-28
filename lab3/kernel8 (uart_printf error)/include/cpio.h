// ref: FreeBSD Manual Pages cpio
/*
General Format: header + pathname + file data + "TRAILER!!!" ("TRAILER!!!" only in the end of archive)
     Each file system object in	a cpio archive comprises a header record with
     basic numeric metadata followed by	the full pathname of the entry and the
     file data.	   The	header is followed by the pathname of the en-
     try (the length of	the pathname is	stored in the header) and any file
     data.  The	end of the archive is indicated	by a special record with the
     pathname "TRAILER!!!".
*/
#ifndef _CPIO_H_
#define _CPIO_H_

//#define CPIO_ADDR ((char*)0x8000000)    //QEMU(0x8000000)
#define CPIO_ADDR ((char*)0x20000000)

/* Magic identifiers for the "New ASCII Format cpio" file format. */
#define CPIO_HEADER_MAGIC "070701"
#define CPIO_FOOTER_MAGIC "TRAILER!!!"	// The end of the archive is indicated by this special record with the pathname
#define CPIO_ALIGNMENT 4				// align 4 bytes

#define CPIO_HEADER_SIZE 110			// 110 bytes
// New ASCII Format: uses 8-byte hexadecimal fields for all numbers
struct cpio_new_header {
    char c_magic[6];      /* Magic header '070701'. */
    char c_ino[8];        /* "i-node" number. */
    char c_mode[8];       /* Permisions. */
    char c_uid[8];        /* User ID. */
    char c_gid[8];        /* Group ID. */
    char c_nlink[8];      /* Number of hard links. */
    char c_mtime[8];      /* Modification time. */
    char c_filesize[8];   /* File size. */
    char c_devmajor[8];   /* Major dev number. */
    char c_devminor[8];   /* Minor dev number. */
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];   /* The number	of bytes in the	pathname that follows the header. This count includes the trailing NUL byte. */
    char c_check[8];      /* Checksum. (This field	is always set to zero by writers and ignored by	readers)*/
};


// extract and convert info from struct cpio_newc_header
struct cpio_size_info {
    unsigned long file_size; 
	unsigned long file_align;
	unsigned long name_size; 
	unsigned long name_align;
    unsigned long offset;

};

void cpio_list();
void cpio_cat(char *args);

#endif /* _CPIO_H_ */
