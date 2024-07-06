#include "scheduler.h"
#include "terminal.h"


// Active process for each terminal, not the actual terminal
int32_t terminal_process_pids[NUM_TERMINALS] = {0,-1,-1};

// Pid for the actual terminals
int32_t terminal_pid[NUM_TERMINALS] = {0,-1,-1};

/*
*   switch_process
*   
*   Input : terminal - specifies which terminal switching to (0-2)
*   Output: 0 - if it worked fine
*           -1 - if it is failed
*   Effect: Pauses current process and switches to process other terminal is running
    
    Notes:
    terminal.h has current_terminal and process_terminal
        -current_terminal indicates which terminal is being displayed
        -process_terminal indicates which terminal whose process is currently active (for scheduling)
    
*/
int32_t switch_process(uint32_t terminal) {
    if (terminal > 2) {return -1;}
    long flags;
    cli_and_save(flags);
    
    // Create a new shell on first time switching to terminal
    if (terminal_pid[terminal] == -1) {
        terminal_pid[terminal] = terminal; 
        process_terminal = terminal;
        restore_flags(flags);
        shell_execute((uint8_t*)"shell");
        return 0;
    }

    // switch to the current process at other terminal
    uint32_t current_pid = terminal_process_pids[terminal];

    /* restore parent data */
    pcb_t * current_PCB = (pcb_t*)(EIGHT_MB - EIGHT_KB * (current_pid + 1));
    set_current_pcb((pcb_t*)(EIGHT_MB - EIGHT_KB * (current_pid + 1)));


    /* restore process paging */
    page_directory[VIRTUAL_ADDR_START >> 22].directory_4MB_entry_desc.offset_bits_31_22 = (KERNEL_MEM_ADDR_END + (current_pid) * PROGRAM_SIZE) >> 22;  //(KERNEL_MEM_ADDR_END >> 22)
    flush_tlb();

    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_MEM_ADDR_END - (current_PCB->pid) * EIGHT_KB - FOUR_B;
    process_terminal = terminal;
    

     /* Restore esp and ebp */
    asm volatile ("                                  \
            movl %0, %%esp                          ;\
            movl %1, %%ebp                          ;\
            "                                                                            
            :                                       
            : "g"(esp_ebp_term[terminal][0]), "g"(esp_ebp_term[terminal][1])  
            : "%esp", "%ebp"          
    );    
    
    restore_flags(flags);
    return 0;
}

/*
*   switch_process_for_sched
*   
*   Input : terminal - specifies which terminal switching to (0-2)
*   Output: 0 - if it worked fine
*           -1 - if it is failed
*   Effect: Pauses current process and switches to process other terminal is running
*/
int32_t switch_process_for_sched(uint32_t terminal) {
    if (terminal > 2) {return -1;}
    long flags;
    cli_and_save(flags);
    
    // Create a new shell on first time switching to terminal
    if (terminal_pid[terminal] == -1) {
        terminal_pid[terminal] = terminal; 
        process_terminal = terminal;
        restore_flags(flags);
        shell_execute((uint8_t*)"shell");
        return 0;
    }

    // switch to the current process at other terminal
    uint32_t current_pid = terminal_process_pids[terminal];

    /* Get pcb */
    set_current_pcb((pcb_t*)(EIGHT_MB - EIGHT_KB * (current_pid + 1)));


    /* restore process paging */
    page_directory[VIRTUAL_ADDR_START >> 22].directory_4MB_entry_desc.offset_bits_31_22 = (KERNEL_MEM_ADDR_END + (current_pid) * PROGRAM_SIZE) >> 22;  //(KERNEL_MEM_ADDR_END >> 22)
    flush_tlb();
    
    process_terminal = terminal;

    restore_flags(flags);
    return current_pid;
}

/*
*   scheduler
*   
*   Input : None
*   Output: None
*   Effect: switches the current process to the next one
*/
void scheduler() {
    if (get_cnt_pid() == 0) return;

    // Context switch
    uint32_t current_pid = switch_process_for_sched((process_terminal + 1) % 3);

    // remap video memory
    remap_vidmap();

    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_MEM_ADDR_END - (current_pid) * EIGHT_KB - FOUR_B;
    

    /* restore esp and ebp */
    asm volatile ("                                  \
            movl %0, %%esp                          ;\
            movl %1, %%ebp                          ;\
            "                                                                            
            :                                       
            : "g"(esp_ebp_term[process_terminal][0]), "g"(esp_ebp_term[process_terminal][1])  
            : "%esp", "%ebp"          
    );    

    return;
    
}

/*
*   remap_vidmap
*   
*   Input : None
*   Output: None
*   Effect: remapping user video memory in virtual address
*/

void remap_vidmap() {
    int flags;
    cli_and_save(flags);

    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.present = 1;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.read_write = 1; 
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.user_supervisor =1; 
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.base_addr = (((uint32_t) user_vidmem_page_table) >> 12);

    if (current_terminal == process_terminal) {
        user_vidmem_page_table[0].base_addr = VID_MEM_ADDR >> 12; // map to video memory
    } else {
        user_vidmem_page_table[0].base_addr = ((VID_MEM_ADDR + ((process_terminal) + 1) * FOUR_KB) >> 12); // map to video page
    }

    user_vidmem_page_table[0].present = 1;
    user_vidmem_page_table[0].read_write = 1;
    user_vidmem_page_table[0].user_supervisor =1; 

    restore_flags(flags);

    flush_tlb();
}

/*
*   get_terminal_pid
*   
*   Input : terminal - specifies which terminal's pid getting
*   Output: returns the pid of the terminal's process
*   Effect: None
*/
int32_t get_terminal_pid(uint32_t terminal) {
    if (terminal > 2) return -1;
    return terminal_pid[terminal];
}


/*
*   get_terminal_process_pid
*   
*   Input : terminal - specifies which terminal's pid getting
*   Output: returns the terminal's process pid
*   Effect: None
*/
int32_t get_terminal_process_pid(uint32_t terminal) {
    if (terminal > 2) return -1;
    return terminal_process_pids[terminal];
}


/*
*   set_terminal_process_pid
*   
*   Input : terminal - specifies which terminal's pid getting
            pid - pid of process terminal is running
*   Output: None
*   Effect: updates the specified terminal_process_pid
*/
void set_terminal_process_pid(uint32_t terminal, uint32_t pid) {
    if (terminal > 2 || pid > MAX_PID) return;
    terminal_process_pids[terminal] = pid;
}
