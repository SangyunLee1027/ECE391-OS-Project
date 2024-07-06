#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "syscall.h"
#include "terminal.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"

#define NUM_TERMINALS   3

// Switch to another terminal process
int32_t switch_process(uint32_t terminal);

// handles scheduling on pit interrupt
void scheduler();

// remapping user video memory in virtual address
void remap_vidmap();

int32_t get_terminal_pid(uint32_t terminal);
int32_t get_terminal_process_pid(uint32_t terminal);

void set_terminal_process_pid(uint32_t terminal, uint32_t pid); // updates the specified terminal_process_pid

#endif
