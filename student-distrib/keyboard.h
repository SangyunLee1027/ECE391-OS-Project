#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "i8259.h"
#include "lib.h"
#include "terminal.h"


#ifndef ASM

// IRQ 1 - keyboard interrupt
// data port 60h
// command port 64h
#define KEYBOARD_IRQ        0x01
#define KEYBOARD_DATA_PORT  0x60
#define KEYBOARD_CMD_PORT   0x64
#define KB_BUFFER_SIZE 128

#define SCANCODE_1_CMD      0xF1
#define SCANCODE_MAX        54


extern int typing_flags[3]; // typing flags for 3 terminals

enum SCS1_SCANCODES {
    EXTENDED = 0xE0,

    P_Q = 0x10,
    P_P = 0x19,
    P_A = 0x1E,
    P_L = 0x26,
    P_Z = 0x2C,
    P_M = 0x32,
    P_1 = 0x02,
    P_EQ = 0x0D,

    P_TILDE = 0x29,
    P_LSH = 0x2A,
    R_LSH = 0xAA,
    P_RSH = 0x36,
    R_RSH = 0xB6,
    P_CAP = 0x3A,
    R_CAP = 0xBA,
    P_BACKSPACE = 0x0E,
    P_TAB = 0x0F,
    P_L_BRACKET = 0x1A,
    P_R_BRACKET = 0x1B,
    P_BSLASH = 0x2B,
    P_ENTER = 0x1C,
    P_COLON = 0x27,
    P_QUOTE = 0x28,
    P_COMMA = 0x33,
    P_PERIOD = 0x34,
    P_FSLASH = 0x35,
    P_ALT = 0x38,
    R_ALT = 0xB8,
    P_SPACE = 0x39,
    P_CTRL = 0x1D,
    R_CTRL = 0x9D,
    P_F1 = 0x3B,
    P_F2 = 0x3C,
    P_F3 = 0x3D,
    P_F4 = 0x3E,
    P_F5 = 0x3F,
    
    P_UP = 0x48,
    P_DOWN = 0x50,
};

// Flag indicating whether a keyboard interrupt has occurred
extern volatile int kb_int_occured;

/* Initialize keyboard */
void keyboard_init(void);

/* Handle keyboard*/
void keyboard_handler(void);

/* Adds character to keyboard buffer*/
int append_key(char c);

/* Removes the last key in keyboard buffer*/
char remove_key(void);

/* returns keyboard buffer*/
char* get_kb_buffer(void);

/* return kb_buf_count*/
int get_kb_buf_count(void);

/* set kb_buf_count*/
void set_kb_buf_count(int n);

int set_kb_buffer(int terminal);

void tab_autocomplete();

#endif
#endif
