#ifndef _TERMINAL_H
#define _TERMINAL_H

#ifndef ASM

#include "keyboard.h"
#include "lib.h"
#include "scheduler.h"

#define TERMINAL_BUFFER_SIZE 128

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7B
#define VIDEO_SIZE  NUM_COLS * NUM_ROWS * 2

// Current terminal being displayed
extern int current_terminal;

// Current terminal that is being processed right now
extern int process_terminal;

// store command history for each terminal (3 terminals)
extern unsigned char command_history[3][50][128];  //3: terminals, 50: number of stored history, 128: TERMINAL_BUFFER_SIZE

// keep track of history row for each terminal (3 terminals)
extern int command_history_row[3];

/*
    Note: text mode is 80x25
*/

/* Initializes terminal*/
void terminal_init();

/* Opens terminal*/
int32_t terminal_open();

/* Closes terminal*/
int32_t terminal_close();

/* Write data to the terminal, displays immediately*/
int32_t terminal_write(int fd, const void* buf, int32_t nbytes);

/* Reads characters previously printed*/
int32_t terminal_read(int fd, void* buf, int32_t nbytes);

/* Clears terminal and resets the buffer*/
void clear_terminal(void);

/* Deletes one character and goes back one space, if at beginning of line go up last line*/
void backspace(void);

/* Prints a char to screen*/
void print_char(uint8_t c);

/* Moves every line up one, and moves cursor to bottom left */
void scroll(void);

void enable_cursor(void);
void disable_cursor(void);

/* Updates position of cursor to current screen_x and screen_y*/
void update_cursor(void);

/* switch terminal display and cursor*/
void switch_terminal(int terminal);

int get_current_terminal();
int get_process_terminal();


extern uint32_t esp_ebp_term[3][2]; // (esp: 0, ebp: 1) for 3 terminals

#endif
#endif
