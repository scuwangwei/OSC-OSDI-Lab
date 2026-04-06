#include "dtb.h"
#include "utils.h"
#include "mini_uart.h"

const char* strings_block;
const char* struct_block;
const char* fdt_end_ptr;
//global var
char *fdt_cpio_addr;

//parse node recursivelly,get property and sub node
void fdt_parse_node(const char** ptr, fdt_callback callback, const char* node_name) {
    while (*ptr < fdt_end_ptr) {
        uint32_t token = be32_to_cpu(*(uint32_t*)(*ptr));
        *ptr += 4;

        if (token == FDT_END_NODE) {
            return;
        } 
        else if (token == FDT_PROP) 
        {
            uint32_t len     = be32_to_cpu(*(uint32_t*)(*ptr)); *ptr += 4;
            uint32_t nameoff = be32_to_cpu(*(uint32_t*)(*ptr)); *ptr += 4;

            const char* prop_name = strings_block + nameoff;
            const void* prop_val  = *ptr;

            *ptr += len;
            *ptr = align(*ptr,4);

            if (callback) callback(node_name, prop_name, prop_val, len);
        }
        else if (token == FDT_BEGIN_NODE)
        {
            const char* child_name = *ptr;
            while (**ptr != '\0') (*ptr)++;
            (*ptr)++;  // skip '\0'
            *ptr = align(*ptr,4);

            fdt_parse_node(ptr, callback, child_name);
        }
        else if (token == FDT_NOP)
        {
            continue;
        }
        else if (token == FDT_END)
        {
            return;
        }
        else
        {
            mini_uart_send_string("Unknown FDT token!\r\n");
            mini_uart_send_hex(token);
            mini_uart_send_string("\r\n");
            return;
        }
    }
}

//get ramfs address
void get_ramfs_addr(const char* node, const char* name, const void* data, uint32_t size) {
    if (strings_compare(name, "linux,initrd-start") == 0 && size == 4) {
        fdt_cpio_addr = (char *)be32_to_cpu(*(uint32_t*)data);
        mini_uart_send_string("initrd-start: ");
        mini_uart_send_hex((unsigned int)fdt_cpio_addr);
        mini_uart_send_string("\r\n");
    }
}

//traverse fdt(dtb),NOTE:Convert big endian to little endian(FTD is big endian but ARM is little endian)
void fdt_traverse(void* fdt_addr, fdt_callback callback) {
    fdt_header_t* hdr = (fdt_header_t*)fdt_addr;

    if (be32_to_cpu(hdr->magic) != FDT_MAGIC) {
        mini_uart_send_string("Invalid FDT magic!\n");
        return;
    }

    strings_block = (const char*)fdt_addr + be32_to_cpu(hdr->off_dt_strings);
    struct_block  = (const char*)fdt_addr + be32_to_cpu(hdr->off_dt_struct);
    fdt_end_ptr   = struct_block + be32_to_cpu(hdr->size_dt_struct);

    const char* ptr = struct_block;
    uint32_t token = be32_to_cpu(*(uint32_t*)ptr);
    ptr += 4;

    if (token == FDT_BEGIN_NODE) {
        const char* root_name = ptr;
        while (*ptr != '\0') ptr++;
        ptr++;  // skip '\0'
        ptr = align(ptr,4);

        fdt_parse_node(&ptr, callback, root_name);
    } else {
        mini_uart_send_string("FDT does not begin with a node!\n");
    }
}

