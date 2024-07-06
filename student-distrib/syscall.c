#include "syscall.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"
#include "keyboard.h"

pcb_t * current_PCB = 0;    // current_PCB = 8MB - 8KB * index
uint32_t current_pid = 0; 
uint32_t cnt_pid = 0;
uint32_t cnt_program = 0;

uint32_t pid_arr[MAX_PID] = {0, 0, 0, 0, 0, 0};

static file_operations_table rtc_op = { rtc_open, rtc_close, rtc_read, rtc_write };
static file_operations_table dir_op = { directory_open, directory_close, directory_read, directory_write };
static file_operations_table file_op = {  file_open, file_close, file_read, file_write };
static file_operations_table terminal_op = { terminal_open, terminal_close, terminal_read, terminal_write };

/*
 * open
 *
 * Input : filename
 * Output: if success return 0, otherwise return -1
 * Effect: provides access to the file system 
 *
 */
int32_t open (const uint8_t* filename)  {
    
    dir_entry_t dentry;    // empty dentry
    int i;
    // If the names file does not exist, the call returns -1
    if (filename == NULL || *filename == '\0' || read_dentry_by_name(filename, &dentry) == -1)  {
        return -1;
    }

    // if not in used, break
    for (i = 0; i < max_file_descriptor; i++) {
        if(current_PCB->file_descriptor_ary[i].flags == 0) { break; }
    }
    if (i == max_file_descriptor) { return -1; } // too many files open
    
    //current_PCB = (pcb_t*)(EIGHT_MB - EIGHT_KB * (current_pid));
    for (i=2; i<max_file_descriptor; ++i)   {
        // assign anunused file descriptor
        if (current_PCB->file_descriptor_ary[i].flags == 0)   { // find the free descriptor (1: in use, 0: free)
            current_PCB->file_descriptor_ary[i].inode = dentry.inode;
            current_PCB->file_descriptor_ary[i].flags = 1;
            current_PCB->file_descriptor_ary[i].file_position = 0;
            // file descriptors need to be set up according to the filetype

            if (dentry.file_type == 0) {   
                current_PCB->file_descriptor_ary[i].file_operations_table_ptr = &rtc_op;
                current_PCB->file_descriptor_ary[i].file_operations_table_ptr->open(0); // initialize rtc interrupt rate to 2 Hz when the RTC device is opened.
            } else if (dentry.file_type == 1) { // if file type is directory
                current_PCB->file_descriptor_ary[i].file_operations_table_ptr = &dir_op;
                current_PCB->file_descriptor_ary[i].file_operations_table_ptr->open(filename); 
            } else if (dentry.file_type == 2) { // if file type is regular file
                current_PCB->file_descriptor_ary[i].file_operations_table_ptr = &file_op;
                current_PCB->file_descriptor_ary[i].file_operations_table_ptr->open(filename); 
            }
            
            return i;
        }
    }
    // no descriptors are free, the call returns -1
    return -1;
}


/*
 * close
 *
 * Input : fd - file descriptor 
 * Output: if success return 0, otherwise return -1
 * Effect: closes the inputted file desciptor and makes it availavle for return from later calls to open
 *
 */
int32_t close (int32_t fd)  {

    // valid file descriptor
    if(fd < 2 || fd >= max_file_descriptor || current_PCB->file_descriptor_ary[fd].flags == 0){
        return -1;
    }

    // set as inactive
    current_PCB->file_descriptor_ary[fd].flags = 0;
    current_PCB->file_descriptor_ary[fd].file_position = 0;
    current_PCB->file_descriptor_ary[fd].inode = 0;
    return current_PCB->file_descriptor_ary[fd].file_operations_table_ptr->close(fd); 
}


/*
 * read
 *
 * Input : fd - file descriptor
 *         buf - buffer
 *         nbytes - nbytes to read
 * Output: if success return 0, otherwise return -1
 * Effect: points to the driver's read function
 *
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes)  {
    // valid file descriptor
    if(fd < 0 || fd >= max_file_descriptor || current_PCB->file_descriptor_ary[fd].flags == 0) {
        return -1;
    }
    
    // points to the driver's read function
    int32_t ret = current_PCB->file_descriptor_ary[fd].file_operations_table_ptr->read(fd, buf, nbytes);
    return ret;
}


/*
 * write
 *
 * Input : fd - file descriptor
 *         buf - buffer
 *         nbytes - nbytes to write
 * Output: if success return 0, otherwise return -1
 * Effect: points to the driver's write function
 *
 *
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes)  {
    // valid file descriptor
    if(fd < 0 || fd >= max_file_descriptor || current_PCB->file_descriptor_ary[fd].flags == 0 || buf == NULL || nbytes < 0){
        return -1;
    }
    // points to the driver's writer function
    int32_t ret = current_PCB->file_descriptor_ary[fd].file_operations_table_ptr->write(fd, buf, nbytes);
    return ret;
}

/*
 * get current pcb
 *
 * Input : None
 * Output: Returns current pcb address
 * Effect: provides access to the file system 
 *
 */
pcb_t* get_current_pcb() {
    int p = get_terminal_process_pid(get_process_terminal());
    if(p < 0){return 0;}
    return (pcb_t*)(EIGHT_MB - EIGHT_KB * (p +1)); 
}


/*
 * set_current_pcb
 *
 * Input : new_pcb - pcb to set to
 * Output: 0 if successful
            -1 otherwise
 * Effect: sets current_PCB to new_pcb 
 *
 */
int32_t set_current_pcb(pcb_t* new_pcb) {
    if (new_pcb == NULL) return -1;
    current_PCB = new_pcb;
    return 0;
}

/*
* halt
*
* Input : status - status
* Output: return a value to the parent execute system call.
* Effect: terminates a process
* 
*/

int32_t halt (uint8_t status) {
    int i;
    
    uint32_t esp_ = current_PCB->saved_esp;
    uint32_t ebp_ = current_PCB->saved_ebp; 
    

    /* Close all processes */
    for (i = 2; i < max_file_descriptor; ++i) {
        close(i);
    }
    
    /* Set currently-active process to non-active */
    current_PCB->active = 0;
    /* Check if main shell */
    if (current_PCB->parent_id == -1) { 
        --cnt_pid;
        pid_arr[current_PCB->pid] = 0;   
        shell_execute((uint8_t*)"shell"); // Restart main shell
        return 0;
    }

    pid_arr[current_PCB->pid] = 0; 

    /* restore parent data */
    current_PCB = (pcb_t*)(EIGHT_MB - EIGHT_KB * (current_PCB->parent_id + 1));
    uint16_t status_ = (uint16_t)status;

    /* restore parent paging */
    page_directory[VIRTUAL_ADDR_START >> 22].directory_4MB_entry_desc.offset_bits_31_22 = (KERNEL_MEM_ADDR_END + (current_PCB->pid) * PROGRAM_SIZE) >> 22;  //(KERNEL_MEM_ADDR_END >> 22)
    flush_tlb();

    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_MEM_ADDR_END - (current_PCB->pid) * EIGHT_KB - FOUR_B;

    --cnt_pid;
    --cnt_program;

    // Update the current terminal's process pid
    set_terminal_process_pid(process_terminal, current_PCB->pid);
    
    typing_flags[process_terminal] = 0;

    /* Halt return (asm) */
    asm volatile ("                                  \
            movl %0, %%esp                          ;\
            movl %1, %%ebp                          ;\
            movl %2, %%eax                          ;\
            "                                                                            
            :                                       
            : "g"(esp_), "g"(ebp_), "g"(status_)     
            : "%eax", "%esp", "%ebp"          
    );     
    
    return (uint32_t)status;
}

/*
 * execute
 *
 * Input : cmd - command
 * Output: return -1 if the command cannot be executed,
 *         return 256 if the progam dies by an exception,
 *         otherwise, if the program executes a halt system call, return a value in the range 0 to 255.
 * Effect: 
 * 
 */
int32_t execute(const uint8_t* command) {
    if(cnt_pid >= MAX_PID || cnt_program >= 3){  //upto 3 programs can run
        return -1;
    }
    int current_esp;
    int current_ebp;
    asm ("movl %%esp, %0;"
         "movl %%ebp, %1;"
         :"=r" (current_esp), "=r" (current_ebp)
    );
    int i;
    uint32_t cmd_length = strlen((int8_t*)command);
    uint8_t command_file_name[FILE_NAME_LENGTH] = "";
    uint8_t* command_temp = (uint8_t*)command; 
    int8_t cmd_copy[cmd_length];
    strcpy(cmd_copy, (int8_t*)command);
    dir_entry_t dentry_temp;

    // uint32_t file_size;
    uint8_t magic_number[4] = {0x7f, 0x45, 0x4c, 0x46}; // magic number to check executable or not
    uint8_t exe_buf[30]; // buffer to check whether the file is executable or not
    uint32_t exe_v_addr = 0;    // take the virtual address from 24-27 in the file
    uint32_t exe_v_addr_page_dir_addr;
    // uint32_t exe_p_addr;    // physical address
    while(*command_temp == ' '){
        ++command_temp;
        --cmd_length;
    }

    /* Parse Command */
    int cmd_end = 0;
    for(i = 0; i < FILE_NAME_LENGTH; ++i)   {
        if(command_temp[i] == ' '){     // the end of file name (space separated)
            
            break;
        }
        else    {
            command_file_name[i] = command_temp[i]; // parse the command
        }
    }
    cmd_end = i + 1;
    
    command_temp = command_temp + cmd_end;
    cmd_length -= cmd_end;
    while(*command_temp == ' '){
        ++command_temp;
        --cmd_length;
        ++cmd_end;
    }

    /* File Checks */
    if (read_dentry_by_name(command_file_name, &dentry_temp) == -1)   {
        return -1;
    }

    if(read_data(dentry_temp.inode, 0, exe_buf, 30) == -1){ // take 30 magic number from the file, if it is not magic number, return -1
        return -1;
    }

    // check magic number
    if(!(exe_buf[0] == magic_number[0] && exe_buf[1] == magic_number[1] && exe_buf[2] == magic_number[2] && exe_buf[3] == magic_number[3])) {
        return -1;
    }

    exe_v_addr = *(uint32_t*)(exe_buf + 24);
    

    for( i =0 ; i < MAX_PID; ++i){
        if(pid_arr[i] == 0){
           current_pid = i;
           pid_arr[i] = 1;
           break; 
        }
    }

    // set up paging for process
    
    exe_v_addr_page_dir_addr = VIRTUAL_ADDR_START >> 22;    // shift 10 bits from 128MB in virtual address to get index

    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.offset_bits_31_22 = (KERNEL_MEM_ADDR_END + current_pid * PROGRAM_SIZE) >> 22; // offset of the physical address
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.present = 1;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.read_write = 1;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.user_supervisor = 1;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.write_through = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.cache_disable = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.accessed = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.dirty = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.page_size = 1;   // the bit is set to exe_v_addr_page_dir_addr, if the mapped page is 4MB in size
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.global_page = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.available = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.page_attrute_table = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.rsvd = 0;
    // flush tlb everytime you start a new process
    flush_tlb();
    


    /* Create New PCB */
    pcb_t* new_pcb = (pcb_t*)(KERNEL_MEM_ADDR_END - (current_pid + 1) * EIGHT_KB);  // since the bottom of the kernel used by kernel, we have to use one above
    new_pcb->pid = current_pid;
    new_pcb->active = 1;

    /* Parse arguments */
    i = 0;
    if (strlen((int8_t*)cmd_copy) > cmd_end) {
        while (*(cmd_copy + cmd_end + i) != '\0') {
            new_pcb->cmd_arg[i] = *(cmd_copy + cmd_end + i);
            ++i;
        }
    }
    
    new_pcb->cmd_arg[i] = 0;
    new_pcb->cmd_arg_len = (i == 0) ? 0 : i + 1;
    
    if (current_pid == 0)   {
        new_pcb->parent_id = -1;
    }
    else {
        new_pcb->parent_id = current_PCB->pid;
        ++cnt_program;
    }
    

    //reset all fd
    for(i = 0; i < max_file_descriptor; ++i){
        new_pcb->file_descriptor_ary[i].flags = 0;
    }

    // stdin & stdout
    for(i=0; i<2; i++)  {
        new_pcb->file_descriptor_ary[i].file_operations_table_ptr = &terminal_op;
        new_pcb->file_descriptor_ary[i].file_position = 0;
        new_pcb->file_descriptor_ary[i].inode = 0;
        new_pcb->file_descriptor_ary[i].flags = 1;
    }

    current_PCB = new_pcb;

    /* Read exe data */
    uint32_t data_len = (index_node_start + dentry_temp.inode)->length;
    uint8_t* program_img_addr =  (uint8_t*)(VIRTUAL_ADDR_START + PROGRAM_IMAGE_ADDR);  // address of this program image
    read_data(dentry_temp.inode, 0, program_img_addr, data_len);
    
    /* Setup old stack & eip */
    tss.ss0 = KERNEL_DS; 
    tss.esp0 = KERNEL_MEM_ADDR_END - (current_pid) * EIGHT_KB - FOUR_B; // esp0 tell the processor where the knernel stack for that pid is. -4 because of index starts from 0 (4 bytes).
    uint32_t user_esp = VIRTUAL_ADDR_START + PROGRAM_SIZE - FOUR_B;
    ++cnt_pid;

    new_pcb->saved_esp = current_esp;
    new_pcb->saved_ebp = current_ebp;
    new_pcb->current_esp = user_esp;
    new_pcb->current_ebp = current_ebp;

    // Update the current terminal's process pid
    set_terminal_process_pid(process_terminal, current_pid);

    // check whether the command needs typing during working
    if(strlen((char*)command_file_name) == 5 && (!strncmp((char*)command_file_name, "shell", 5))){    
        typing_flags[process_terminal] = 0;
    }   else if(strlen((char*)command_file_name) == 5 && (!strncmp((char*)command_file_name, "hello", 5))){
        typing_flags[process_terminal] = 0;
    }   else if(strlen((char*)command_file_name) == 7 && (!strncmp((char*)command_file_name, "counter", 7))){
        typing_flags[process_terminal] = 0;
    }  else{
        typing_flags[process_terminal] = 1;
    }

    /* Go to usermode (IRET) */
    asm volatile ("                     \
            movl    %0, %%eax           ;\
            movw    %%ax, %%ds          ;\
            pushl   %0                  ;\
            pushl   %1                  ;\
            pushfl                      ;\
            pushl   %2                  ;\
            pushl   %3                  ;\
            iret                        ;\
            "
            :                           
            : "g"(USER_DS), "g"(user_esp), "g"(USER_CS), "g"(exe_v_addr)
    );  
    return 0;
}


/*
 * shell_execute
 *
 * Input : cmd - command
 * Output: return -1 if the command cannot be executed,
 *         return 256 if the progam dies by an exception,
 *         otherwise, if the program executes a halt system call, return a value in the range 0 to 255.
 * Effect: 
 * 
 */
int32_t shell_execute(const uint8_t* command) {
    if(cnt_pid >= MAX_PID){
        return -1;
    }
    int flags;
    cli_and_save(flags);

    int current_esp;
    int current_ebp;
    asm ("movl %%esp, %0;"
         "movl %%ebp, %1;"
         :"=r" (current_esp), "=r" (current_ebp)
    );
    int i;
    uint32_t cmd_length = strlen((int8_t*)command);
    uint8_t command_file_name[FILE_NAME_LENGTH] = "";
    uint8_t* command_temp = (uint8_t*)command; 
    int8_t cmd_copy[cmd_length];
    strcpy(cmd_copy, (int8_t*)command);
    dir_entry_t dentry_temp;



    // uint32_t file_size;
    uint8_t magic_number[4] = {0x7f, 0x45, 0x4c, 0x46}; // magic number to check executable or not
    uint8_t exe_buf[30]; // buffer to check whether the file is executable or not
    uint32_t exe_v_addr = 0;    // take the virtual address from 24-27 in the file
    uint32_t exe_v_addr_page_dir_addr;
    // uint32_t exe_p_addr;    // physical address
    while(*command_temp == ' '){
        ++command_temp;
        --cmd_length;
    }

    /* Parse Command */
    int cmd_end = 0;
    for(i = 0; i < FILE_NAME_LENGTH; ++i)   {
        if(command_temp[i] == ' '){     // the end of file name (space separated)
            
            break;
        }
        else    {
            command_file_name[i] = command_temp[i]; // parse the command
        }
    }
    cmd_end = i + 1;
    
    command_temp = command_temp + cmd_end;
    cmd_length -= cmd_end;
    while(*command_temp == ' '){
        ++command_temp;
        --cmd_length;
        ++cmd_end;
    }

    /* File Checks */
    if (read_dentry_by_name(command_file_name, &dentry_temp) == -1)   {
        return -1;
    }

    if(read_data(dentry_temp.inode, 0, exe_buf, 30) == -1){ // take 30 magic number from the file, if it is not magic number, return -1
        return -1;
    }

    // check magic number
    if(!(exe_buf[0] == magic_number[0] && exe_buf[1] == magic_number[1] && exe_buf[2] == magic_number[2] && exe_buf[3] == magic_number[3])) {
        return -1;
    }

    exe_v_addr = *(uint32_t*)(exe_buf + 24);
    

    for( i =0 ; i < MAX_PID; ++i){
        if(pid_arr[i] == 0){
           current_pid = i;
           pid_arr[i] = 1;
           break; 
        }
    }

    restore_flags(flags);

    // set up paging for process
    
    exe_v_addr_page_dir_addr = VIRTUAL_ADDR_START >> 22;    // shift 10 bits from 128MB in virtual address to get index

    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.offset_bits_31_22 = (KERNEL_MEM_ADDR_END + current_pid * PROGRAM_SIZE) >> 22; // offset of the physical address
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.present = 1;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.read_write = 1;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.user_supervisor = 1;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.write_through = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.cache_disable = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.accessed = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.dirty = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.page_size = 1;   // the bit is set to exe_v_addr_page_dir_addr, if the mapped page is 4MB in size
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.global_page = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.available = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.page_attrute_table = 0;
    page_directory[exe_v_addr_page_dir_addr].directory_4MB_entry_desc.rsvd = 0;
    // flush tlb everytime you start a new process
    flush_tlb();
    
    cli_and_save(flags);


    /* Create New PCB */
    pcb_t* new_pcb = (pcb_t*)(KERNEL_MEM_ADDR_END - (current_pid + 1) * EIGHT_KB);  // since the bottom of the kernel used by kernel, we have to use one above
    new_pcb->pid = current_pid;
    new_pcb->active = 1;

    /* Parse arguments */
    i = 0;
    if (strlen((int8_t*)cmd_copy) > cmd_end) {
        while (*(cmd_copy + cmd_end + i) != '\0') {
            new_pcb->cmd_arg[i] = *(cmd_copy + cmd_end + i);
            ++i;
        }
    }
    
    new_pcb->cmd_arg[i] = 0;
    new_pcb->cmd_arg_len = (i == 0) ? 0 : i + 1;
    
 
    new_pcb->parent_id = -1;

    

    //reset all fd
    for(i = 0; i < max_file_descriptor; ++i){
        new_pcb->file_descriptor_ary[i].flags = 0;
    }

    // stdin & stdout
    for(i=0; i<2; i++)  {
        new_pcb->file_descriptor_ary[i].file_operations_table_ptr = &terminal_op;
        new_pcb->file_descriptor_ary[i].file_position = 0;
        new_pcb->file_descriptor_ary[i].inode = 0;
        new_pcb->file_descriptor_ary[i].flags = 1;
    }

    current_PCB = new_pcb;

    /* Read exe data */
    uint32_t data_len = (index_node_start + dentry_temp.inode)->length;
    uint8_t* program_img_addr =  (uint8_t*)(VIRTUAL_ADDR_START + PROGRAM_IMAGE_ADDR);  // address of this program image
    read_data(dentry_temp.inode, 0, program_img_addr, data_len);
    
    /* Setup old stack & eip */
    tss.ss0 = KERNEL_DS; 
    tss.esp0 = KERNEL_MEM_ADDR_END - (current_pid) * EIGHT_KB - FOUR_B; // esp0 tell the processor where the knernel stack for that pid is. -4 because of index starts from 0 (4 bytes).
    uint32_t user_esp = VIRTUAL_ADDR_START + PROGRAM_SIZE - FOUR_B;
    ++cnt_pid;

    new_pcb->saved_esp = current_esp;
    new_pcb->saved_ebp = current_ebp;
    new_pcb->current_esp = user_esp;
    new_pcb->current_ebp = current_ebp;

    // Update the current terminal's process pid
    set_terminal_process_pid(process_terminal, current_pid);

    restore_flags(flags);

    /* Go to usermode (IRET) */
    asm volatile ("                     \
            movl    %0, %%eax           ;\
            movw    %%ax, %%ds          ;\
            pushl   %0                  ;\
            pushl   %1                  ;\
            pushfl                      ;\
            pushl   %2                  ;\
            pushl   %3                  ;\
            iret                        ;\
            "
            :                           
            : "g"(USER_DS), "g"(user_esp), "g"(USER_CS), "g"(exe_v_addr)
    );  
    return 0;
}

/*
*   getargs
*   
*   Input : buf - buffer storing the args
*           nbytes - number of bytes to write into
*   Output : none
*/
int32_t getargs(uint8_t* buf, int32_t nbytes){
    if (buf == NULL || current_PCB->cmd_arg_len > nbytes ||
    current_PCB->cmd_arg_len <= 0) return -1;

    // copy args over to buf
    int i = 0;
    while (current_PCB->cmd_arg[i] != 0 && i < nbytes) {
        buf[i] = current_PCB->cmd_arg[i];
        ++i;
    }
    buf[i] = 0;
    return 0;
}

/*
*   vidmap
*   
*   Input : screen_start - pointer of video memory address
*   Output: 0 - if it worked fine
*           -1 - if it is failed
*/
int32_t vidmap(uint8_t ** screen_start){
    if((uint32_t)screen_start == NULL || (uint32_t)screen_start < VIRTUAL_ADDR_START || (uint32_t)screen_start > VIRTUAL_ADDR_START + PROGRAM_SIZE){
        return -1;
    }
    
    int flags;
    cli_and_save(flags);


    *screen_start = (uint8_t *)USER_VIDMEM_ADDR;
    
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.present = 1;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.read_write = 1; 
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.user_supervisor =1; 
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.write_through =0;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.cache_disable =0;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.accessed = 0;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.dirty =0;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.page_size = 0;   // the bit is set to 0, if the mapped page is 4KB in size
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.global_page =0;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.available =0;
    page_directory[USER_VIDMEM_IDX].directory_4KB_entry_desc.base_addr = (((unsigned int) user_vidmem_page_table) >> 12);
    
    user_vidmem_page_table[0].present = 1;
    user_vidmem_page_table[0].read_write = 1;
    user_vidmem_page_table[0].user_supervisor =1; 
    user_vidmem_page_table[0].write_through =0;
    user_vidmem_page_table[0].cache_disable =0;
    user_vidmem_page_table[0].accessed = 0;
    user_vidmem_page_table[0].dirty = 0;
    user_vidmem_page_table[0].table_attr_idx = 0;
    user_vidmem_page_table[0].global_page = 0;
    user_vidmem_page_table[0].available = 0;

    if (current_terminal == process_terminal) {
        user_vidmem_page_table[0].base_addr = VID_MEM_ADDR >> 12;
    } else {
        user_vidmem_page_table[0].base_addr = ((VID_MEM_ADDR + (process_terminal + 1) * FOUR_KB) >> 12);
    }
    
    restore_flags(flags);

    flush_tlb();
    
    return 0;

}


/*
*   set_handler
*   
*   Input : signum - specifies which signal's hander to change
*           handler_address - pointer to user-level function to handle signal
*   Output: 0 - if it worked fine
*           -1 - if it is failed
*   Effect: Updates a signal's handler function (were it actually implemented)
*/
int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}

/*
*   sigreturn
*   
*   Input : none
*   Output: 0 - if it worked fine
*           -1 - if it is failed
*   Effect: copies hardware context back on processor
*/
int32_t sigreturn(void) {
    return -1;
}

/*
*   flush tlb
*
*   Input : None
*   Output: None
*/
void flush_tlb() {
    asm volatile ("                 \n\
        movl    %%cr3, %%eax          \n\
        movl    %%eax, %%cr3          \n\
        "    
        :
        :
        : "eax"         
    );
    return;
}

int32_t get_cnt_pid(){
    return cnt_pid;
}
