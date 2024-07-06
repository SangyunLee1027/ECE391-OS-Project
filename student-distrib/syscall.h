#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "rtc.h"
#include "file_system.h"
#include "x86_desc.h"

#define EIGHT_MB 0x800000   // 8MB
#define EIGHT_KB 0x2000     // 8KB
#define FOUR_MB 0x400000   // 4MB
#define VIRTUAL_ADDR_START 0x8000000 // 128MB

#define FOUR_B 4
#define FOUR_KB 4096

#define max_file_descriptor 8   // each task can have up to 8 open files
#define TERMINAL_MAX_SIZE   128 // max size of terminal

#define KERNEL_MEM_ADDR_END     0x800000 // kernel end address
#define MAX_PID                 6     // max num of processes allowed
#define PROGRAM_SIZE            0x400000    // each program size 4MB
#define PROGRAM_IMAGE_ADDR      0x48000     // the offset of the program image

// #define USER_VIDMEM_ADDR        0x8800000   // 136 MB

// page faults
// #define USER_VIDMEM_ADDR        0x8000000 + FOUR_MB * (MAX_PID + 1)   // 128 + MAX_PID * 4 MB

#define USER_VIDMEM_ADDR           0x9C00000 //128 + MAX_PID * 4 MB

#define USER_VIDMEM_IDX         USER_VIDMEM_ADDR/FOUR_MB // 136MB/4MB


/* jump table  (file operaions table) */ 
typedef struct file_operations_table {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} file_operations_table;

/* file descriptor */
typedef struct file_descriptor_entry_t{
    file_operations_table* file_operations_table_ptr; 
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags; // whether it is open or not
} file_descriptor_entry_t;

/* pcb */
typedef struct pcb_t{
    uint32_t pid; // process id
    uint32_t parent_id;
    file_descriptor_entry_t file_descriptor_ary[max_file_descriptor];    // file_descriptor_ary
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t current_esp;
    uint32_t current_ebp;
    uint8_t active;
    uint32_t cmd_arg_len;
    uint8_t cmd_arg[TERMINAL_MAX_SIZE];
} pcb_t;

extern void syscall_handler();

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);

int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);

int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t ** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

int32_t shell_execute(const uint8_t* command);

// helper functions
extern pcb_t* get_current_pcb();
extern int32_t set_current_pcb(pcb_t* new_pcb);
void flush_tlb();
int32_t get_cnt_pid();

#endif /* _SYSCALL_H */
