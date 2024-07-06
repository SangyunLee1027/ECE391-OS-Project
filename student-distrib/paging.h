 #ifndef PAGING_H
 #define PAGING_H


#include "types.h"

 #ifndef ASM


#define MAX_ENTRY 1024
#define FOUR_K_BYTE 4096    //to make least significant 12 bits to 0, so we can use it for features
#define KERNEL_ADDR  0x400000    // The kernel is loaded at physical address 0x400000 (4 MB), and also mapped at virtual address 4 MB. (from Appendix C)
#define VID_MEM_ADDR 0xB8000      //the address of the video memory


typedef union page_directory_entry_t{

    struct {
        uint32_t present               : 1;
        uint32_t read_write            : 1;
        uint32_t user_supervisor       : 1;
        uint32_t write_through         : 1;
        uint32_t cache_disable         : 1;
        uint32_t accessed              : 1;
        uint32_t dirty                 : 1;
        uint32_t page_size             : 1;
        uint32_t global_page           : 1;
        uint32_t available             : 3;
        uint32_t page_attrute_table    : 1;
        uint32_t rsvd                  : 9; // processor deteched 1s in reserved bits of the page directory (IA32-ref-manual-vol-3 pg.188)
        uint32_t offset_bits_31_22     : 10;
    } directory_4MB_entry_desc __attribute__ ((packed));;

    struct {
        uint32_t present               : 1;
        uint32_t read_write            : 1;
        uint32_t user_supervisor       : 1;
        uint32_t write_through         : 1;
        uint32_t cache_disable         : 1;
        uint32_t accessed              : 1;
        uint32_t dirty                 : 1;
        uint32_t page_size             : 1;
        uint32_t global_page           : 1;
        uint32_t available             : 3;
        uint32_t base_addr             : 20;
    } directory_4KB_entry_desc __attribute__ ((packed));

}   page_directory_entry_t;

typedef struct table_entry_desc {
    uint32_t present               : 1;
    uint32_t read_write            : 1;
    uint32_t user_supervisor       : 1;
    uint32_t write_through         : 1;
    uint32_t cache_disable         : 1;
    uint32_t accessed              : 1;
    uint32_t dirty                 : 1;
    uint32_t table_attr_idx        : 1;
    uint32_t global_page           : 1;
    uint32_t available             : 3;
    uint32_t base_addr             : 20;
} page_table_entry_t;


page_directory_entry_t page_directory[MAX_ENTRY] __attribute__((aligned(FOUR_K_BYTE)));
page_table_entry_t video_page_table[MAX_ENTRY] __attribute__((aligned(FOUR_K_BYTE)));
page_table_entry_t user_vidmem_page_table[MAX_ENTRY] __attribute__((aligned(FOUR_K_BYTE))); // page table for video memory for user

extern void page_add();
extern void page_init();
extern void loadPageDirectory(page_directory_entry_t*);
extern void enablePaging();
extern void page_terminal_vidmem(int terminal);

#endif
#endif
