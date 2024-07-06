#include "paging.h"

#define ASM     0



/*
    page_init
    Description: Initialize the paging for the video memory and kernel
    Input: none
    Output: none
    Effects: initializes paging for video memory and kernel
*/
void page_init() {
    unsigned int i;
    unsigned int j;

    for (i = 0; i < MAX_ENTRY; i++) {
        page_directory[i].directory_4KB_entry_desc.present =0;
        page_directory[i].directory_4KB_entry_desc.read_write = 1;
        page_directory[i].directory_4KB_entry_desc.user_supervisor =0; 
        page_directory[i].directory_4KB_entry_desc.write_through =0;
        page_directory[i].directory_4KB_entry_desc.cache_disable =0;
        page_directory[i].directory_4KB_entry_desc.accessed = 0;
        page_directory[i].directory_4KB_entry_desc.dirty =0;
        page_directory[i].directory_4KB_entry_desc.page_size = 0;   // the bit is set to 0, if the mapped page is 4KB in size
        page_directory[i].directory_4KB_entry_desc.global_page =0;
        page_directory[i].directory_4KB_entry_desc.available =0;
        page_directory[i].directory_4KB_entry_desc.base_addr =0;
    }


    for (j = 0; j < MAX_ENTRY; j++) {
        video_page_table[j].present = 0;
        video_page_table[j].read_write = 1;
        video_page_table[j].user_supervisor =0; 
        video_page_table[j].write_through =0;
        video_page_table[j].cache_disable =0;
        video_page_table[j].accessed = 0;
        video_page_table[j].dirty = 0;
        video_page_table[j].table_attr_idx = 0;
        video_page_table[j].global_page = 0;
        video_page_table[j].available = 0;
        video_page_table[j].base_addr = j;
    }

    /* set up for video memory address*/
    page_directory[0].directory_4KB_entry_desc.present = 1;
    page_directory[0].directory_4KB_entry_desc.read_write = 1; 
    page_directory[0].directory_4KB_entry_desc.user_supervisor =0; 
    page_directory[0].directory_4KB_entry_desc.write_through =0;
    page_directory[0].directory_4KB_entry_desc.cache_disable =0;
    page_directory[0].directory_4KB_entry_desc.accessed = 0;
    page_directory[0].directory_4KB_entry_desc.dirty =0;
    page_directory[0].directory_4KB_entry_desc.page_size = 0;   // the bit is set to 0, if the mapped page is 4KB in size
    page_directory[0].directory_4KB_entry_desc.global_page =0;
    page_directory[0].directory_4KB_entry_desc.available =0;
    page_directory[0].directory_4KB_entry_desc.base_addr = (((unsigned int) video_page_table) >> 12);

    /* set page for video memory*/
    int vid_mem_idx = VID_MEM_ADDR >> 12; // need to right shift by 12 to find the address of the table (Table addr = 20~12 bit of linear address)
    video_page_table[vid_mem_idx].present = 1;
    video_page_table[vid_mem_idx].base_addr = VID_MEM_ADDR >> 12;   // 20 most significant bits


    /* set up for kernel */
    page_directory[1].directory_4MB_entry_desc.present = 1;
    page_directory[1].directory_4MB_entry_desc.read_write = 1;
    page_directory[1].directory_4MB_entry_desc.user_supervisor = 0;
    page_directory[1].directory_4MB_entry_desc.write_through = 0;
    page_directory[1].directory_4MB_entry_desc.cache_disable = 0;
    page_directory[1].directory_4MB_entry_desc.accessed = 0;
    page_directory[1].directory_4MB_entry_desc.dirty = 0;
    page_directory[1].directory_4MB_entry_desc.page_size = 1;   // the bit is set to 1, if the mapped page is 4MB in size
    page_directory[1].directory_4MB_entry_desc.global_page = 0;
    page_directory[1].directory_4MB_entry_desc.available = 0;
    page_directory[1].directory_4MB_entry_desc.page_attrute_table = 0;
    page_directory[1].directory_4MB_entry_desc.rsvd = 0;
    page_directory[1].directory_4MB_entry_desc.offset_bits_31_22 = KERNEL_ADDR >> 22;   // 10 most significant bits

    loadPageDirectory(page_directory);
    enablePaging();
}


/*
    page_terminal_vidmem
    Description: Initialize the paging for the video memory of terminals
    Input: none
    Output: none
*/
void page_terminal_vidmem(int terminal){
    int vid_mem_idx = (VID_MEM_ADDR + terminal * FOUR_K_BYTE) >> 12; // need to right shift by 12 to find the address of the table (Table addr = 20~12 bit of linear address)

    video_page_table[vid_mem_idx].base_addr = vid_mem_idx;   // 20 most significant bits
    video_page_table[vid_mem_idx].present = 1;
}
