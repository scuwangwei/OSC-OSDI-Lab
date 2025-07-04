#ifndef CPIO_H
#define CPIO_H


//#define CPIO_ARCHIVE_ADDR 0x20000000 comment this.because will get the address from fdt

//new ASCII format cpio archive
typedef struct {
    char c_magic[6]; //070701 refers new ASCII format
    //rest of those all use ASCII(char) to store hex
    char c_ino[8];//inode
    char c_mode[8];//file type and permission
    char c_uid[8];//user id
    char c_gid[8];//group id
    char c_nlink[8];//hard link count
    char c_mtime[8];//last modified time in UNIX timestamp
    char c_filesize[8];//file size (byte)
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
} cpio_newc_header;

#define SIZE_OF_CPIO_HEADER (sizeof(cpio_newc_header))
void cpio_ls();
void cpio_cat();
char *cpio_findFile(const char *fileName, int *fileSize);
void cpio_load_program();
#endif