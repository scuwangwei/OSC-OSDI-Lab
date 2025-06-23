#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

#define FDT_MAGIC       0xd00dfeed
#define FDT_BEGIN_NODE  0x1
#define FDT_END_NODE    0x2
#define FDT_PROP        0x3
#define FDT_NOP         0x4
#define FDT_END         0x9

typedef struct {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} fdt_header_t;

typedef void (*fdt_callback)(const char* node_name, const char* prop_name, const void* prop_val, uint32_t prop_len);
void fdt_traverse(void* fdt_addr, fdt_callback callback);
void get_ramfs_addr(const char* node, const char* name, const void* data, uint32_t size);
void fdt_parse_node(const char** ptr, fdt_callback callback, const char* node_name);
#endif