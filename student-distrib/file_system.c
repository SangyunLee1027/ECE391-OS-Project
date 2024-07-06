#include "file_system.h"
#include "syscall.h"

uint32_t inode_index; // index to print files in directories


/* file_sys_init
 *
 * Input :  boot_block_addr - the address where we set the boot_block
 * Output: None
 * Effect: Initialize the file system (ptr for the boot block, the first index node, and the first data block)
 */

void file_sys_init(uint32_t boot_block_addr){

    boot_block_ptr = (boot_block_t*) boot_block_addr;   // initialize boot block at mod-mod_start
    index_node_start = (index_node_t*) (boot_block_ptr + 1);    // index node starts at (where boot block) + 1 : right after boot block
    data_block_start = (data_block_t*) (index_node_start + boot_block_ptr->num_inodes); // data block starts at (where index node starts) + (# of inodes)
    inode_index = 0;

}


/* read_dentry_by_name
 *
 * Input : fname  : file name
 *         dentry : address of dentry block where we copy the file we are looking for
 * Output: If success, return 0. Otherwise (a non-existent file or invalid index), return -1.
 * Effect: copy the file of the inputted file name into the dentry block  
 */

int32_t read_dentry_by_name (const uint8_t* fname, dir_entry_t* dentry){
    int i;
    
    uint32_t fname_len = strlen((int8_t*)fname);
    
    if(fname_len > FILE_NAME_LENGTH){     // keep file name 32B max
        return -1;
    }

    for(i = 0; i < boot_block_ptr->num_inodes; ++i){    // scan through the directory entries in the boot block to find the file name
        if(!strncmp((int8_t*)fname, (int8_t*)(boot_block_ptr->d_entry[i].file_name), FILE_NAME_LENGTH)){  // check 32 letters
            read_dentry_by_index(i, dentry);    // populate the dentry paramter
            return 0;
        }
    }

    return -1;
}


/* read_dentry_by_index
 *
 * Input : index  : index node
 *         dentry : address of dentry block where we copy the file we are looking for
 * Output: If success, return 0. Otherwise (a non-existent file or invalid index), return -1.
 * Effect: copy the file of the inputted index file into the dentry block  
 */

int32_t read_dentry_by_index (uint32_t index, dir_entry_t* dentry){
    
    if(index > boot_block_ptr->num_inodes || index < 0){    // check index within the range
        printf("failed read index");
        return -1;
    }

    // populate the dentry paramter
    strcpy(dentry->file_name, boot_block_ptr->d_entry[index].file_name);    // populate the file name
    dentry->file_type = boot_block_ptr->d_entry[index].file_type;           // populate the file type
    dentry->inode = boot_block_ptr->d_entry[index].inode;                   // populate the inode number
    return 0;
}


/* read_data
 * 
 * Input : inode  : index node of the starting position
 *         offset : file address of the starting position
 *         buf    : a buffer where the read data would be placed
 *         length : number of bytes to read
 * Output: If the offset is not within the range, return -1.
 *         If it reaches the end of the file, return 0.
 *         Otherwise, return the number of bytes read.
 * Effect: The buffer store the copy of the read data within the given length of bytes
 *
 */

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    if (offset < 0) {   // check if offset within the range
        return -1;
    }

    int i;

    uint32_t num_byte_counter = 0;
    
    index_node_t* cur_inode = index_node_start + inode;  // address of the current inode

    uint32_t file_length = cur_inode->length;

    if(file_length <= offset){ // check the offset within the range
        return 0;
    }

    uint32_t data_block_index = offset / DATA_BLOCK_SIZE;   // index node of the starting position
    
    uint32_t data_offset = offset % DATA_BLOCK_SIZE;    // offset (xth data block #) of the starting position

    data_block_t* cur_data_block = data_block_start + (cur_inode->data_blocks[data_block_index++]);  // address of the current data block

    uint32_t current_pos = data_offset; // current position of the data block -> keep track of determining if it reads the whole data block (move on to next data block or not)

    for (i = 0; i < length; ++i)    {   // looping up to length bytes
        
        buf[i] = cur_data_block->data_[current_pos++];  // place current data in the buffer

        ++num_byte_counter;

        if(num_byte_counter + offset >= file_length){    // if the end of the file has been reached, break
            buf[i] = '\n';
            break;
        }

        if(current_pos >= DATA_BLOCK_SIZE){ // if it reached the end of data block, move to the next data block
            current_pos = 0;
            cur_data_block = data_block_start + (cur_inode->data_blocks[data_block_index++]);    // address of the data block adter moving to the next data block
        }
        
    }
    
    return num_byte_counter;  // return the number of bytes read
}


/* dir_read
 * 
 * Input : fd  : file descriptor
 *         buf    : a buffer where the read data would be placed
 *         nbytes : how many bytes to read
 * Output: If the offset is not within the range, return -1.
 *         If it reaches the end of the file, return 0.
 *         Otherwise, return the number of bytes read.
 * Effect: The buffer store the the string to print which shows the file name, file type, and file size.
 *
 */

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    
    int i, j; 

    dir_entry_t cur_dir = boot_block_ptr->d_entry[inode_index]; // keep track of current directory entry

    char str1[] = "file_name: ";
    memcpy(buf, str1, 10);
    buf  = (void*)((uint32_t)buf + 10); // go next

    char str2[32] = "                                ";
    uint32_t name_len = strlen(cur_dir.file_name);
    if(name_len > 32){
        name_len = 32;  // keep file name length 32B max
    }
    for(i = 32-name_len, j = 0; i < 32; ++i){  // copy the name by looping through each characters of the file name
        str2[i] = cur_dir.file_name[j++];
    }

    memcpy(buf, str2, 32);
    buf = (void*)((uint32_t)buf + 32);

    char str3[13] = ", file_type: ";
    memcpy(buf, str3, 13);
    buf = (void*)((uint32_t)buf + 13);
    
    *(char*)buf = (char)(cur_dir.file_type + '0');
    buf = (void*)((uint32_t)buf + 1);

    char str4[13] = ", file_size: ";
    memcpy(buf, str4, 13);
    buf = (void*)((uint32_t)buf + 13);


    if(cur_dir.file_type != 2){     // file type {0: user-level access to RTC / 1: directory}
        char str6[6] = "     0";
        memcpy(buf, str6, 6);
    }   else {  // file type {2: regular file}
        index_node_t* cur_inode = index_node_start + (cur_dir.inode);  // address of the current inode

        uint32_t file_size = cur_inode->length;
        char str5[6] = "      ";

        for(i=0; i < 6; ++i){
            if(file_size == 0){
                break;
            }
            str5[5-i] = (file_size%10) + '0';
            file_size /= 10;
        }

        memcpy(buf, str5, 6);
    }

    ++inode_index;

    if(inode_index >= boot_block_ptr->num_dir_entries){     // when reach the end directory entry, reset the inode index
        inode_index = 0;
    }

    return 0;
}

/* get_num_dir_entry
 * 
 * Input : None
 * Output: return the number of directory entries in boot block.
 * Effect: None
 *
 */

uint32_t get_num_dir_entry(){
    return boot_block_ptr->num_dir_entries;     // return the number of directory entries in boot block.
}


/* file_open
 * 
 * Input : filename
 * Output: If success, return 0. If failed, return -1.
 * Effect: Provides access to file system. Find directory entry corresponding to names file, allocate an unused fd, and set up data necessary
 *
 */

int32_t file_open(const uint8_t* filename) {
    // There's notthing to be done here, since everything is being set in syscall open() already
    return 0;  
}


/* file_close
 * 
 * Input : None
 * Output: If success, return 0. If failed, return -1.
 * Effect: None
 *
 */

int32_t file_close(int32_t fd) {
    pcb_t* current_PCB = get_current_pcb();
    current_PCB->file_descriptor_ary[fd].flags = 0; // set file flag to free:0
    current_PCB->file_descriptor_ary[fd].file_position = 0;
    return 0;
}

/* file_read
 * 
 * Input : fd - file descriptor
 *         buf - a buffer where the read data would be placed
 *         nbytes - number of bytes to read from file
 * Output: return the number of bytes read; if initial pos at/beyond end of file, return 0. If ailed, return -1.
 * Effect: Reads data from file; should be read to the end of the file or the end of the buffer provided, whichever occurs sooner.
 *
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {   // read only system
    int ret;
    if (buf == NULL) return -1;
    
    pcb_t* current_PCB = get_current_pcb(); // get current pcb address
    ret = read_data(current_PCB->file_descriptor_ary[fd].inode, current_PCB->file_descriptor_ary[fd].file_position, buf, nbytes);
    current_PCB->file_descriptor_ary[fd].file_position += ret;

    return ret;
}


/* file_write
 * 
 * Input : None
 * Output: Always return -1 since it is a read-only system
 * Effect: None
 *
 */

int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


/* directory_open
 * 
 * Input : None
 * Output: If success, return 0.
 * Effect: None
 *
 */

int32_t directory_open(const uint8_t* filename) {
    dir_entry_t dentry;

    if(read_dentry_by_name(filename, &dentry) || dentry.file_type != 1) {
        return -1;
    }
    return 0;  
}

/* directory_close
 * 
 * Input : None
 * Output: If success, return 0.
 * Effect: None
 *
 */

int32_t directory_close() {
    return 0;
}

/* directory_read
 * 
 * Input : None
 * Output: return the number of bytes read; if initial pos at/beyond end of file, return 0.
 * Effect: subsequenc reads should read from successive directory entries until last is reaches, -> repeatedly return 0.
 *
 */

int32_t directory_read(int32_t fd, void* buf, int32_t nbytes) { 
    if(fd < 0 || fd > 7 || buf == NULL) { return 0; }

    pcb_t * current_pcb = get_current_pcb();
    if(current_pcb == NULL) { return 0; }
    if(current_pcb->file_descriptor_ary[fd].file_position > boot_block_ptr->num_inodes) { return 0; }

    int8_t * target = (int8_t*)buf;
    int8_t * temp = (int8_t*)boot_block_ptr->d_entry[current_pcb->file_descriptor_ary[fd].file_position].file_name;
    int i = 0;

    while (temp[i] != '\0' && i < FILE_NAME_LENGTH)
    {
        target[i] = temp[i];
        i++;
    }
    current_pcb->file_descriptor_ary[fd].file_position++;
    return i;

}


/* directory_write
 * 
 * Input : None
 * Output: return -1 since it is a read only system
 * Effect: None
 *
 */

int32_t directory_write() {
    return -1;  // read-only system
}

