#include "keyboard.h"

char keycodes[SCANCODE_MAX][2] = {   
    { 0X0, 0X0 },                   
    { 0x0, 0x0 }, { '1', '!' }, // ESC
    { '2', '@' }, { '3', '#' }, 
    { '4', '$' }, { '5', '%' }, 
    { '6', '^' }, { '7', '&' }, 
    { '8', '*' }, { '9', '(' }, 
    { '0', ')' }, { '-', '_' }, 
    { '=', '+' }, { 0x8, 0x8 }, // BACKSPACE
    { 0x9, 0x9 }, { 'q', 'Q' }, // TAB
    { 'w', 'W' }, { 'e', 'E' }, 
    { 'r', 'R' }, { 't', 'T' }, 
    { 'y', 'Y' }, { 'u', 'U' }, 
    { 'i', 'I' }, { 'o', 'O' }, 
    { 'p', 'P' }, { '[', '{' }, 
    { ']', '}' }, { 0xA, 0xA }, // ENTER 
    { 0x0, 0x0 }, { 'a', 'A' }, // L/R CTRL
    { 's', 'S' }, { 'd', 'D' }, 
    { 'f', 'F' }, { 'g', 'G' }, 
    { 'h', 'H' }, { 'j', 'J' }, 
    { 'k', 'K' }, { 'l', 'L' }, 
    { ';', ':' }, { '\'', '"' }, 
    { '`', '~'}, { 0x0, 0x0 }, //SHIFT 
    { '\\', '|' }, { 'z', 'Z' }, 
    { 'x', 'X' }, { 'c', 'C' }, 
    { 'v', 'V' }, { 'b', 'B' }, 
    { 'n', 'N' }, { 'm', 'M' }, 
    { ',', '<'}, { '.', '>' },
    { '/', '?' },
};

int typing_flags[3] = {0, 0, 0}; // typing flags for 3 terminals


unsigned prev_key = 0;
char capslock_on = 0;

char lshift_on = 0;
char rshift_on = 0;

char lctrl_on = 0;
char rctrl_on = 0;

char lalt_on = 0;
char ralt_on = 0;

char kb_buf1[KB_BUFFER_SIZE];
char kb_buf2[KB_BUFFER_SIZE];
char kb_buf3[KB_BUFFER_SIZE];
char* terminal_buffers[3] = {kb_buf1, kb_buf2, kb_buf3};
unsigned kb_buf_len[3] = {0,0,0};

char *kb_buffer = kb_buf1;
static int kb_buf_count = 0;

int command_history_cnt[3] = {0, 0 , 0};

/*
    keyboard init
    Input: none
    Output: none
    Effects: initializes keyboard by enabling IRQ 1
*/
void keyboard_init(void) {
    kb_int_occured = 0;
    kb_buf_count = 0;
    prev_key = 0;
    capslock_on = 0;
    lshift_on = 0;
    rshift_on = 0;
    lctrl_on = 0;
    rctrl_on = 0;
    lalt_on = 0;
    ralt_on = 0;
    kb_buffer = kb_buf1;
    memset(kb_buf1,0,KB_BUFFER_SIZE);
    memset(kb_buf2,0,KB_BUFFER_SIZE);
    memset(kb_buf3,0,KB_BUFFER_SIZE);
    
    outb(SCANCODE_1_CMD, KEYBOARD_CMD_PORT);
    enable_irq(KEYBOARD_IRQ);
}

/*
    keyboard handler
    keyboard interrupts are generated on press and release   
    Input: none
    Output: none
    Effects: handles keyboard interrupts, 
            if key can be written, writes to screen and buffer
*/
void keyboard_handler(void) {
    long flags;

    cli_and_save(flags);    //disable interrupt

    uint8_t scan_code = inb(KEYBOARD_DATA_PORT); // get keyboard input
    int pt = process_terminal;
    process_terminal = current_terminal;
    if (scan_code == EXTENDED) { // int is first part of two byte long scancode
        goto keyboard_eoi;
    }


    if (prev_key == EXTENDED) { // int is second part of two byte long scancode
        if (scan_code == P_ALT) {
            ralt_on = 1;
        } else if (scan_code == R_ALT) {
            ralt_on = 0;
        }

        if (scan_code == P_CTRL) {
            rctrl_on = 1;
        } else if (scan_code == R_CTRL) {
            rctrl_on = 0;
        }

        goto keyboard_eoi;
    }

    // Switch to different terminal
    if ((ralt_on || lalt_on) && scan_code >= P_F1 && scan_code <= P_F3) {
        
        int terminal = scan_code - P_F1; // 0-2

        // do nothing if trying to switch to current terminal
        if (terminal == current_terminal) goto keyboard_eoi;
        process_terminal = pt;
        
        set_kb_buffer(terminal);
        prev_key = scan_code;
        kb_int_occured = 1;
        send_eoi(KEYBOARD_IRQ);
        
        restore_flags(flags); //enable interrupt 
        
        switch_terminal(terminal); 
        return;
    }

    if (scan_code == P_UP)   {   // check up and down for history command
        if(typing_flags[process_terminal]){
            goto keyboard_eoi;
        }
        int i = 0;
        if(command_history_cnt[current_terminal] < command_history_row[current_terminal]){
            command_history_cnt[current_terminal]++;
            clear_row();
            while(remove_key());
            while (command_history[current_terminal][command_history_row[current_terminal] - command_history_cnt[current_terminal]][i] != '\n')  {
                append_key(command_history[current_terminal][command_history_row[current_terminal] - command_history_cnt[current_terminal]][i]);
                putc(command_history[current_terminal][command_history_row[current_terminal] - command_history_cnt[current_terminal]][i]);
                i++;
            }
        }
        goto keyboard_eoi;
    }

    if (scan_code == P_DOWN)   {   // check up and down for history command
        if(typing_flags[process_terminal]){
            goto keyboard_eoi;
        }
        int i = 0;
        if(command_history_cnt[current_terminal] > 1){
            command_history_cnt[current_terminal]--;
            clear_row();
            while(remove_key());
            while (command_history[current_terminal][command_history_row[current_terminal] - command_history_cnt[current_terminal]][i] != '\n')  {
                append_key(command_history[current_terminal][command_history_row[current_terminal] - command_history_cnt[current_terminal]][i]);
                putc(command_history[current_terminal][command_history_row[current_terminal] - command_history_cnt[current_terminal]][i]);
                i++;
            }
        }   else if(command_history_cnt[current_terminal] == 1){
            clear_row();
            while(remove_key());
        }
        goto keyboard_eoi;
    }

    if (scan_code == P_BACKSPACE) { // check back space
        if (kb_buf_count > 0) {
            if (remove_key() == '\t') {
                backspace();
                backspace();
                backspace();
                backspace();
            }
            backspace();
        }
    }

    // tab to autocomplete
    if (scan_code == P_TAB) {
        if(typing_flags[process_terminal]){
            goto keyboard_eoi;
        }
        tab_autocomplete();
    }

    if (scan_code == P_SPACE) { // check spacebar
        if(typing_flags[process_terminal]){
            goto keyboard_eoi;
        }
        if (append_key(' '))
            {print_char(' ');}
    }

    if (scan_code == P_ENTER) { // check enter
        if(typing_flags[process_terminal]){
            goto keyboard_eoi;
        }
        if (append_key('\n')){
            print_char('\n');
            command_history_cnt[current_terminal] = 0;
            memcpy(command_history[current_terminal][command_history_row[current_terminal]], terminal_buffers[current_terminal], strlen(terminal_buffers[current_terminal]));
            command_history_row[current_terminal]++;
        }
            
        goto keyboard_eoi;
    }

    if (scan_code == P_LSH) { // check left shift
        lshift_on = 1;
    } else if(scan_code == R_LSH) {
        lshift_on = 0;
    }

    if (scan_code == P_RSH) { // check right shift
        rshift_on = 1;
    } else if(scan_code == R_RSH) {
        rshift_on = 0;
    }

    if (scan_code == P_ALT) {
        lalt_on = 1;
    } else if (scan_code == R_ALT) {
        lalt_on = 0;
    }

    if (scan_code == P_CTRL) {  // check left control
        lctrl_on = 1;
    } else if (scan_code == R_CTRL) {
        lctrl_on = 0;
    }

    if (scan_code == P_CAP) { // check caps lock
        capslock_on = (capslock_on) ? 0 : 1;
        goto keyboard_eoi;
    } 


    if (lctrl_on || rctrl_on) {
        if (scan_code == P_L) {
            clear_terminal();            // clear screen
            kb_buf_count = 0;   // clear keyboard buffer
            
        } 
        goto keyboard_eoi;
    }

    char let_pressed =  (scan_code >= P_Q && scan_code <= P_P) ||
                        (scan_code >= P_A && scan_code <= P_L) ||
                        (scan_code >= P_Z && scan_code <= P_M);

    if (let_pressed) { // valid letter key
        if(typing_flags[process_terminal]){
            goto keyboard_eoi;
        }
        char key = keycodes[scan_code][((lshift_on | rshift_on) || capslock_on) ? 1 : 0];
        if (append_key(key)) {
            print_char(key); // print key pressed
            goto keyboard_eoi;
        }
        
        
    } 
    
    char special_pressed =  (scan_code >= P_1 && scan_code <= P_EQ) ||
                            (scan_code == P_L_BRACKET) || (scan_code == P_R_BRACKET) ||
                            (scan_code == P_BSLASH) ||
                            (scan_code == P_TILDE) ||
                            (scan_code == P_COLON || scan_code == P_QUOTE) ||
                            (scan_code >= P_COMMA && scan_code <= P_FSLASH); // ||
                            // (scan_code == P_TAB);
    
    if (special_pressed) { // valid number key
        if(typing_flags[process_terminal]){
            goto keyboard_eoi;
        }
        char key = keycodes[scan_code][(lshift_on | rshift_on) ? 1 : 0];
        if (append_key(key)) {
            print_char(key); // print key 
        }
    }
keyboard_eoi:
    prev_key = scan_code;
    kb_int_occured = 1;
    send_eoi(KEYBOARD_IRQ);
    process_terminal = pt;
    restore_flags(flags); //enable interrupt  
    return;

}

/*
    Adds given character to keyboard buffer
    Input: c - char to be appended
    Output: 1 if sucessful, 0 otherwise
    Effects: updates keyboard_buffer
*/
int append_key(char c) {

    if (kb_buf_count >= (KB_BUFFER_SIZE-1)) { // buffer is full already

        if (c == '\n') { // only newline can go into buffer now
            kb_buffer[KB_BUFFER_SIZE - 1] = c;
            return 1;
        }
        return 0;

    } else {
        if (kb_buf_count < 0) kb_buf_count = 1;

        kb_buffer[kb_buf_count++] = c;
    }
    
    return 1;
}

/*
    Removes the most recent character from the keyboard buffer
    Input: c - char to be appended
    Output: The char removed, 0 otherwise
    Effects: updates keyboard_buffer
*/
char remove_key(void) {
    if (kb_buf_count >= KB_BUFFER_SIZE) { // if buffer is full
        kb_buf_count = KB_BUFFER_SIZE;
    }
    if (kb_buf_count <= 0) return 0;    // buffer is empty

    char r = kb_buffer[--kb_buf_count];
    kb_buffer[kb_buf_count] = 0;
    return r;
}

/*
    Returns pointer to keyboard buffer
    Input: none
    Output: kb_buffer
    Effects: none
*/
char* get_kb_buffer(void) {
    return kb_buffer;
}

/*
    Returns keyboard buffer count
    Input: none
    Output: kb_buf_count
    Effects: none
*/
int get_kb_buf_count(void) {
    return kb_buf_count;
}

/*
    Sets kb_buf_count to value lower than current
    Input: n - new count
    Output: none
    Effects: kb_buf_count changes, removes n least recent keys in keyboard buffer
*/
void set_kb_buf_count(int n) {
    if (n <= 0) {kb_buf_count = 0; memset(kb_buffer,0,KB_BUFFER_SIZE); return;}
    if (n >= kb_buf_count){return;}
    int i;
    unsigned difference = kb_buf_count - n;
    for (i = 0; i < n; ++i) {
        kb_buffer[i] = kb_buffer[i + difference];
    }

    kb_buf_count = n;
}


/*
    Set kb_buffer to new buffer
    Input: new_buffer - new buffer with size of at least KB_BUFFER_SIZE
    Output: 0 if sucessful, -1 otherwise
    Effects: kb_buffer now using a different buffer
*/
int set_kb_buffer(int terminal) {
    if (terminal > 2 || terminal < 0) return -1;
    kb_buf_len[current_terminal] = kb_buf_count; // save buffer length

    // switch buffer
    kb_buffer = terminal_buffers[terminal]; 
    kb_buf_count = kb_buf_len[terminal];

    // check because why not
    if (kb_buf_count > KB_BUFFER_SIZE) kb_buf_count = KB_BUFFER_SIZE;

    return 0;
}

/*
    tab_autocomplete
    Input: None
    Output: autocompleted command/argument
    Effects: autocompletes commands or argument on terminal
*/
void tab_autocomplete(void) {
    if (kb_buf_count >= (FILE_NAME_LENGTH)) { return; } // if current typed command greater than max file name length, return
    // to store current typed arg
    char curr_typed[FILE_NAME_LENGTH + 1];

    // int i = 0; // index

    int32_t read_count = 0;
    int32_t buf_read_count = 0;

    char search_buf[FILE_NAME_LENGTH + 1];
    char target_buf[(FILE_NAME_LENGTH + 1) * max_file_descriptor];

    int32_t target_buf_has = 0;
    int32_t target_len = 0;

    while(kb_buf_count != buf_read_count) {
        /* if ' ' read, reset and redetect arg*/
        if(kb_buffer[buf_read_count] == ' ') {
            buf_read_count++;
            read_count = 0;
            continue;
        }

        /* copy into current typed buffer */
        curr_typed[read_count] = kb_buffer[buf_read_count];
        buf_read_count++;
        read_count++;
    }

    if(read_count > FILE_NAME_LENGTH) { return; }
    curr_typed[read_count] = '\0'; // add empty char to the end of buffer

    int32_t file_read_count = 0;
    int32_t curr_file_len = 0;


    while (0 != (curr_file_len = directory_read(0, search_buf, FILE_NAME_LENGTH + 1))) {
        file_read_count++;
        if (0 == strncmp(search_buf, curr_typed, read_count)) {
            if(target_buf_has == 1) { break; } // if more than one possibility, go with first one...
            strncpy(target_buf, search_buf, FILE_NAME_LENGTH + 1);
            target_buf_has = 1;
            target_len = curr_file_len;
        }
    }

    pcb_t * current_pcb = get_current_pcb();
    current_pcb->file_descriptor_ary[0].file_position = 0;

    // where to auto complete
    int32_t complete_pos = 0;
    while(target_buf[complete_pos] == curr_typed[complete_pos]) {
        complete_pos++;
    }

    /* auto complete */
    int32_t curr_pos = complete_pos;
    while(curr_pos < target_len) {
        print_char(target_buf[curr_pos]);
        kb_buffer[kb_buf_count++] = target_buf[curr_pos];
        curr_pos++;
        /* return if buffer is full */
        if(kb_buf_count == (KB_BUFFER_SIZE - 1)) { return; }
    }

    
}
