#include "cpio.h"
#include "mini_uart.h"
#include "utils.h"
#include "dtb.h"
#include "time.h"

#define MAX_READER_BUFFER 64
#define USER_PROGRAM_LOAD_ADDR  0x400000
#define USER_PROGRAM_STACK_ADDR 0x800000
extern char *fdt_cpio_addr;
unsigned long user_entry;
unsigned long user_stack;

/*
┌────────────────────────┐ ← ptr = CPIO_ARCHIVE_ADDR
│ cpio_newc_header       │ ← fixed 110 bytes (each field is ASCII hex format)
└────────────────────────┘
┌────────────────────────┐
│ filename (nameSize)    │ ← after header
│ (zero-padded if needed)│
└────────────────────────┘
┌────────────────────────┐
│ file data (fileSize)   │ ← after header
│ (zero-padded if needed)│
└────────────────────────┘
↑
│
│ repeat till name == "TRAILER!!!"

file data is 4 bytes align

*/

/*
return value
    0 : invalid cpio magic number
    -1 : fild not found
    fileData : file raw data
*/
char *cpio_findFile(const char *fileName, int *fileSize)
{
    //get cpio archive start address
    char *ptr = fdt_cpio_addr;
    while (1)
    {
        cpio_newc_header *header = (cpio_newc_header *)ptr;
        if (strcmp(header->c_magic, "070701",6))
        {
            mini_uart_send_string("Invalid cpio magic number!\r\n");
            return 0;
        }

        char *name = (char *)header + SIZE_OF_CPIO_HEADER;
        int name_size = hex2int(header->c_namesize, 8);
        int file_size = hex2int(header->c_filesize, 8);

        // end of archive
        if (strings_compare(name, "TRAILER!!!") == 0)
        {
            mini_uart_send_string("File not found!\r\n");
            return 0;
        }

        // found the file
        if (strings_compare(name, fileName) == 0)
        {
            //get file data and make sure it is aligned
            char *fileData = (char *)(((unsigned long)(name + name_size) + 3) & ~3UL);
            *fileSize = file_size;
            return fileData;
        }

        // Align file data pointer to 4 bytes after file name
        char *fileData = (char *)(((unsigned long)(name + name_size) + 3) & ~3UL);
        // Align next header pointer to 4 bytes after file data
        ptr = (char *)(((unsigned long)(fileData + file_size) + 3) & ~3UL);
    }
}

void cpio_ls()
{
    char *ptr = fdt_cpio_addr;
    while(1)
    {
        cpio_newc_header *header = (cpio_newc_header *)ptr;
        if (strcmp(header->c_magic, "070701",6))
        {
            mini_uart_send_string("Invalid cpio magic number!\r\n");
            break;
        }

        char *name = (char *)header + SIZE_OF_CPIO_HEADER;
        int nameSize = hex2int(header->c_namesize, 8);
        int fileSize = hex2int(header->c_filesize, 8);

        // end of archive
        if (strings_compare(name, "TRAILER!!!") == 0) break;
        
        //get file name
        mini_uart_send_string(name);
        mini_uart_send_string("\r\n");
        

        // Align file data pointer to 4 bytes after file name
        char *fileData = (char *)(((unsigned long)(name + nameSize) + 3) & ~3UL);
        // Align next header pointer to 4 bytes after file data
        ptr = (char *)(((unsigned long)(fileData + fileSize) + 3) & ~3UL);
    }
}

void cpio_cat()
{
    mini_uart_send_string("Filename: ");
    mini_uart_send_string("\r\n");
    char buffer[MAX_READER_BUFFER];
    char c;
    int i = 0;
    while (1)
    {
        c = mini_uart_read();
        if (c == '\n') {
            buffer[i] = '\0';
            break;
        }
        mini_uart_write(c); // echo back
        buffer[i++] = c;
        if (i >= MAX_READER_BUFFER - 1) {
            buffer[i] = '\0';
            break;
        }
    }
    mini_uart_send_string("\r\n");
    int fileSize;
    char *fileData = cpio_findFile(buffer,&fileSize);

    if(fileData)
    {
        //just in case fileData is not ended with \0, so i get file size first and print file data
        for (int j = 0; j < fileSize; j++) 
        {
            if (fileData[j] == '\n') {
                mini_uart_send_string("\r\n");
            }
            else mini_uart_write(fileData[j]);
        }
        mini_uart_send_string("\r\n");
    }
}

void cpio_load_program()
{
    mini_uart_send_string("Please enter file name: ");
    char fileName[MAX_READER_BUFFER];
    mini_uart_read_string(fileName,MAX_READER_BUFFER);
    //get program start address in initramfs
    int fileSize;
    char *file_addr = cpio_findFile(fileName,&fileSize);
    if(!file_addr)
    {
        mini_uart_send_string("Program not found\r\n");
        return;
    }
    else
    {
        mini_uart_send_string("Loading program: ");
        mini_uart_send_string(fileName);
        mini_uart_send_string("\r\n");
        //declare a taget address where the program will be loaded at
        char *target_addr = (char *)(USER_PROGRAM_LOAD_ADDR);
        
        //set user program entry point and stack pointer address
        user_entry = (unsigned long)USER_PROGRAM_LOAD_ADDR;
        user_stack = (unsigned long)USER_PROGRAM_STACK_ADDR;

        //loading(copying) data
        for(int i=0;i<fileSize;i++)
        {
            target_addr[i] = file_addr[i];
        }

        mini_uart_send_string("Jumping to user program (EL0)...\r\n");

        asm volatile(
            "mov x0, 0x340  \n"
            "msr spsr_el1, x0   \n"
        );
        asm volatile("msr elr_el1, %0    \n" ::"r"(user_entry));
        asm volatile("msr sp_el0, %0    \n" ::"r"(user_stack));
        asm volatile("eret   \n");
    }
    
} 