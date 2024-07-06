#include "terminal.h"

// static int screen_x;
// static int screen_y;

int terminal_cursor_positions[NUM_TERMINALS][2] = { {0,0},{0,0},{0,0} };

static char* video_mem = (char *)VIDEO;

int x_pos1[NUM_ROWS];
int x_pos2[NUM_ROWS];
int x_pos3[NUM_ROWS];
int* terminal_x_positions[NUM_TERMINALS] = {x_pos1, x_pos2, x_pos3};
int *x_positions = x_pos1;

volatile int kb_int_occured = 0;

int current_terminal = 0;
int process_terminal = 0;
uint32_t esp_ebp_term[NUM_TERMINALS][2] = {{-1,-1}, {-1, -1}, {-1,-1}}; // array to store esp and ebp before switch the process

int cursor_on = 0;

int command_history_row[3] = {0,0,0};   // initialie the command history row to 0
unsigned char command_history[3][50][128];
/*
    Initializes and opens the terminal
    Input: none
    Output: returns 0 for success, -1 if failed
    Effects: Opens terminal and file descriptor pointing to terminal
*/
void terminal_init() {
    kb_int_occured = 0;
    screen_x = 0;
    screen_y = 0;
    kb_int_occured = 0;
    cursor_on = 0;
    x_positions = x_pos1;
    current_terminal = 0;
    process_terminal = 0;
    int i,j;
    for (i = 0; i < NUM_TERMINALS; ++i) {
        for (j = 0; j < 2; ++j) {
            terminal_cursor_positions[i][j] = 0;
        }
    }
    memset(x_pos1, 0, NUM_ROWS);
    memset(x_pos2, 0, NUM_ROWS);
    memset(x_pos3, 0, NUM_ROWS);
    
    page_terminal_vidmem(1);
    page_terminal_vidmem(2);
    page_terminal_vidmem(3);

    for (i = 0; i < 50; i++)    { // 50: number of stored history
        for (j = 0; j < TERMINAL_BUFFER_SIZE; j++)  {
            command_history[0][i][j] = NULL;
            command_history[1][i][j] = NULL;
            command_history[2][i][j] = NULL;
        }
    }

    flush_tlb();

}

/*
    Initializes and opens the terminal
    Input: none
    Output: returns 0 for success, -1 if failed
    Effects: Opens terminal and file descriptor pointing to terminal
*/
int32_t terminal_open(void) {
    terminal_init();
    return 0;
}

/*
    Resets and closes the terminal
    Input: none
    Output: returns 0 for success, -1 if failed
    Effects: Closes terminal and clears related variables
*/
int32_t terminal_close(void) {
    disable_cursor();
    return 0;
}

/*
    Writes nbytes bytes from buf to screen 
    Input: buf - buffer to display
            nbytes - number of bytes from buf to display
    Output: number of bytes sucessfully written, or -1 if failed to write
    Effects: Displays buf to terminal
*/
int32_t terminal_write(int fd, const void* buf, int32_t nbytes) {

    // NULL check    
    if (buf == 0 || fd != 1) {return -1;}
    long flags;
    uint32_t r = 0;
    int i;

    char* b = (char*)buf;
    char c;

    for (i = 0; i < nbytes; ++i) {
       
        cli_and_save(flags);    //disable interrupt
        c = (char)b[r];
        
        print_char(c);
        ++r;

        restore_flags(flags); //enable interrupt 
        
    }

    return r;
}

/*
    Reads from input device and places into buf   
    Returns only when enter key if pressed
    Input: buf - buffer
            nbytes - number of bytes to ready from buf
    Output: number of bytes sucessfully read, -1 if failed
    Effects: Updates line_buffer
*/
int32_t terminal_read(int fd, void* buf, int32_t nbytes) {

    // NULL check
    if (buf == 0 || fd != 0) return -1;
    long flags;
    uint32_t bytes_read = 0;
    
    char* b = (char*)buf;
    int kb_count, pos;

    // First check if the keyboard buffer contains an enter key
    // If so copy that first segment
    cli_and_save(flags);    //disable interrupt
    char* kb_buf = get_kb_buffer();
    kb_count = get_kb_buf_count();
    
    // If nothing in keyboard don't bother
    if (kb_count > 0 && process_terminal == current_terminal) {
        
        // Check to what point it can copy from keyboard
        for (bytes_read = 0; bytes_read < nbytes && bytes_read < kb_count; ++bytes_read) {
            
            // Break if there is a newline
            if (kb_buf[bytes_read] == '\n' || kb_buf[bytes_read] == 0) {break;}
            
        }

        // // Convert from index to actual bytes read
        if (b[bytes_read] != '\n') b[bytes_read] = '\n';
        bytes_read++;
        b[bytes_read] = 0;
        memcpy(b, kb_buf, bytes_read);
        set_kb_buf_count(kb_count - bytes_read); // update keyboard buffer
        restore_flags(flags); //enable interrupt
        return bytes_read;
    }
    

    restore_flags(flags); //enable interrupt


    /*
        In this case user still needs to type until enter or reached nbytes typed
        user calls read, this is called
        Reads from keyboard buffer until either newline in it or buffer is filled (nbytes)
        When either happens copy bytes over to buf arg including the '\n'
        Return bytes read from keyboard buffer
    */
    while (1) {

        // Wait until an int has occured, then check if int was keyboard
        while (kb_int_occured == 0 || process_terminal != current_terminal) {asm volatile ("hlt");} 
        
        cli_and_save(flags);    //disable interrupt
        
        kb_int_occured = 0;
        kb_buf = get_kb_buffer();
        kb_count = get_kb_buf_count();
        // check if last key was newline
        if (kb_buf[kb_count-1] == '\n' || kb_buf[KB_BUFFER_SIZE-1] == '\n') { 
            pos = 0;
            // write to buf until reached newline, end of keyboard buffer or requested nbytes
            while (pos < kb_count && pos < nbytes) {
                b[pos] = kb_buf[pos];
                bytes_read++;
                if (b[pos++] == '\n') { // last read was newline
                    break;
                }

            }

            // ensure buf ends with a newline
            if (b[pos-1] != '\n') b[pos++] = '\n';
            b[pos] = 0;
            set_kb_buf_count(kb_count - bytes_read); // update keyboard buffer
            restore_flags(flags); //enable interrupt
            return bytes_read;
        }
        restore_flags(flags); //enable interrupt
    }
    
    return bytes_read;
}


/*
    Clears the terminal and buffer
    Input: none
    Output: none
    Effects: Clears terminal and resets buffer, text position goes back to top left
*/
void clear_terminal(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) { // Clear each char
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    screen_x = 0;
    screen_y = 0;
    update_cursor();
    return;
}

/*
    Goes back one character
    If backspace at beginning of line, go up one line
    Input: none
    Output: none
    Effects: Removes most recent character from screen
*/
void backspace(void) {
    if (--screen_x < 0) {
        x_positions[screen_y] = 0;
        if (--screen_y < 0) {
            screen_y = 0;
            screen_x = 0; 
            return;
        }
        screen_x = x_positions[screen_y];
    }
    // clears char where the cursor is at now
    *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + (screen_x)) << 1)) = ' ';
    *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + (screen_x)) << 1) + 1) = ATTRIB;
    update_cursor();
}


/*
    prints char to screen
    Input: none
    Output: none
    Effects: prints given char to screen at cursor position, and moves cursor
*/
void print_char(uint8_t c) {

    char* vidmem;
    int sx;
    int sy;
    int *x_pos;
    long flags;
    cli_and_save(flags);
    // printing to the video page it should be printed to
    if (process_terminal == current_terminal || process_terminal < 0) {
        vidmem = video_mem;
        sx = screen_x;
        sy = screen_y;
        x_pos = x_positions;
    } else {
        vidmem = (char*) (VIDEO + (process_terminal + 1) * FOUR_KB);
        sx = terminal_cursor_positions[process_terminal][0];
        sy = terminal_cursor_positions[process_terminal][1];
        x_pos = terminal_x_positions[process_terminal];
    }
    
    if(c == '\n' || c == '\r') {
        x_pos[sy++] = sx;
        sx = 0;
    } else if (c == '\t') {
        sx += 4;
        goto print_over_cols_check;

    } else if (c == '\b') {
        backspace();
    } else {
        
        // Show character to screen
        *(uint8_t *)(vidmem + ((NUM_COLS * sy + (sx)) << 1)) = c;
        *(uint8_t *)(vidmem + ((NUM_COLS * sy + (sx)) << 1) + 1) = ATTRIB;

print_over_cols_check:
        // If cursor need to go to new line
        if (++sx >= NUM_COLS) {
            x_pos[sy++] = sx;
            sx = 0;
        }

    }
    
    if (process_terminal == current_terminal) {
        screen_x = sx; screen_y = sy;
        update_cursor();
    } else {
        terminal_cursor_positions[process_terminal][0] = sx;
        terminal_cursor_positions[process_terminal][1] = sy;
        terminal_x_positions[process_terminal] = x_pos;
    }

    // If screen needs to scroll to get a new blank line
    if (sy >= NUM_ROWS) {
        scroll();
    }
    restore_flags(flags);
    return;
}

/*
    Moves every line up one, and moves cursor to bottom left 
    Input: none
    Output: none
    Effects: Scrolls screen vertically down
*/
void scroll(void) {
    uint32_t i;
    uint8_t* d, *s;

    static char* vidmem;
    int sy;
    int *x_pos = terminal_x_positions[process_terminal];

    // using video page it should be printed to
    if (process_terminal == current_terminal) {
        vidmem = video_mem;
        sy = screen_y;
    } else {
        vidmem = (char*) (VIDEO + (process_terminal + 1) * FOUR_KB);
        sy = terminal_cursor_positions[process_terminal][1];
    }
    

    for (i = 0; i < NUM_ROWS-1; ++i) {
        x_pos[i] = x_pos[i+1];
        d = (uint8_t *)(vidmem + ((NUM_COLS * i * 2)));
        s = (uint8_t *)(vidmem + ((NUM_COLS * (i+1) * 2)));
        memmove(d, s, NUM_COLS * 2);
    }
    
    uint32_t r = (NUM_COLS * (NUM_ROWS-1));
    // Clear bottom row
    for (i = 0; i < NUM_COLS; ++i) {
        *(uint8_t *)((vidmem + (r + i) * 2)) = ' ';
        *(uint8_t *)((vidmem + (r + i) * 2) + 1) = ATTRIB;
    }

    if (process_terminal == current_terminal) {
        screen_x = 0;
        screen_y = NUM_ROWS-1;
        update_cursor();
    } else {
        terminal_cursor_positions[process_terminal][0] = 0;
        terminal_cursor_positions[process_terminal][1] = NUM_ROWS-1;
        
    }
}



#define CRCT_PORT1 0x3D4
#define CRCT_PORT2 0x3D5
#define CURSOR_INIT_REG1    0x0A
#define CURSOR_INIT_REG2    0x0B
#define CURSOR_INIT1        0xC0
#define CURSOR_INIT2        0x10 //0xE0
#define CURSOR_REG1 0x0F
#define CURSOR_REG2 0x0E
#define CURSOR_DISABLE  0x20

#define CURSOR_SCANLINE_S   10
#define CURSOR_SCANLINE_E   11

//enable cursor
void enable_cursor(void) {

    outb(CURSOR_INIT_REG1,CRCT_PORT1);
    uint8_t t = inb(CRCT_PORT2) & CURSOR_INIT1; // Set Cursor to enabled
    outb(CURSOR_INIT_REG1, CRCT_PORT1);
    outb((t) | (CURSOR_SCANLINE_S), CRCT_PORT2);       // Cursor scanline start
    
    outb(CURSOR_INIT_REG2, CRCT_PORT1);
    t = inb(CRCT_PORT2) & CURSOR_INIT2;         // Sets Cursor Skew aka offset
    outb(CURSOR_INIT_REG2, CRCT_PORT1);
    outb((t) | (CURSOR_SCANLINE_E), CRCT_PORT2);       // Cursor scanline end
    cursor_on = 1;
    update_cursor();
}

// disable cursor
void disable_cursor(void) {
    cursor_on = 0;
    outb(CURSOR_INIT_REG1, CRCT_PORT1);
    outb(CURSOR_DISABLE, CRCT_PORT2);
}

/*
    Updates position of cursor to current screen_x and screen_y
    Input: none
    Output: none
    Effects: Changes cursor position
*/
void update_cursor(void) {

    if (!cursor_on) return;
    uint32_t pos = (screen_y * NUM_COLS  + screen_x); // pos
    outb(CURSOR_REG1, CRCT_PORT1);
    outb((uint8_t)(pos & 0xFF), CRCT_PORT2); // first half of position

    outb(CURSOR_REG2, CRCT_PORT1);
    outb((uint8_t)( (pos >> 8) & 0xFF), CRCT_PORT2); // second half
}

/*
    Updates terminal
    Input: terminal - terminal number to switch to
    Output: none
    Effects: Saves current terminal's screen, switches screen to new terminal,
             Changes cursor position
*/
void switch_terminal(int terminal) {
    if (terminal < 0 || terminal > 2) return;
    if (terminal == current_terminal) return;
    long flags;
    cli_and_save(flags);
    // save
    terminal_cursor_positions[current_terminal][0] = screen_x;
    terminal_cursor_positions[current_terminal][1] = screen_y;

    screen_x = terminal_cursor_positions[terminal][0];
    screen_y = terminal_cursor_positions[terminal][1];

    x_positions = terminal_x_positions[terminal];
    char* current_vid_page = (char*) (VIDEO + (current_terminal + 1) * FOUR_KB);
    char* next_vid_page = (char*) (VIDEO + (terminal + 1) * FOUR_KB);
    memcpy(current_vid_page, video_mem, VIDEO_SIZE);
    memcpy(video_mem, next_vid_page, VIDEO_SIZE);
    
    current_terminal = terminal;

    update_cursor();
    restore_flags(flags);
}

int get_current_terminal() {
    return current_terminal;
}

int get_process_terminal() {  
    return process_terminal;
}
