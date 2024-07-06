#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "lib.h"
#include "types.h"


#define FILE_NAME_LENGTH            32
#define BOOT_BLOCK_RESERVED_SIZE    52
#define DIR_ENTRY_RESERVED_SIZE     24        
#define DATA_BLOCKS_MAX_NUM         1023
#define BOOT_BLOCK_DIR_ENTRY_SIZE   63
#define DATA_BLOCK_SIZE             4096

typedef struct dir_entry_t{
    char file_name[FILE_NAME_LENGTH]; 
    int file_type;
    uint32_t inode;
    uint8_t reserved[DIR_ENTRY_RESERVED_SIZE];
} dir_entry_t;

typedef struct boot_block_t{
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[BOOT_BLOCK_RESERVED_SIZE];
    dir_entry_t d_entry[BOOT_BLOCK_DIR_ENTRY_SIZE];
}   boot_block_t;

typedef struct index_node_t{
    uint32_t length;
    uint32_t data_blocks[DATA_BLOCKS_MAX_NUM];
}   index_node_t;

typedef struct data_block_t{
    uint8_t data_[DATA_BLOCK_SIZE];
}   data_block_t;

// initialize file system
void file_sys_init(uint32_t boot_block_addr);

// copy the file of the inputted file name into the dentry block
int32_t read_dentry_by_name (const uint8_t* fname, dir_entry_t* dentry);

// copy the file of the inputted index file into the dentry block
int32_t read_dentry_by_index (uint32_t index, dir_entry_t* dentry);

// store the copy of the read data within the given length of bytes into the buffer
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// store the the string to print which shows the file name, file type, and file size into the buffer
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

// return the number of directory entries in boot block
uint32_t get_num_dir_entry();

int32_t file_open(const uint8_t* filename);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t directory_open(const uint8_t* filename);
int32_t directory_close();
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_write();

boot_block_t* boot_block_ptr;
index_node_t* index_node_start;
data_block_t* data_block_start;

#endif
