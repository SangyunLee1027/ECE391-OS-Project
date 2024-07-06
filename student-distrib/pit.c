#include "pit.h"
#include "terminal.h"

/*
    pit_init
    Input: None
    Output: none
    Effects: initialized pit
*/
void pit_init() {
    long flags;
    cli_and_save(flags);
    outb(PIT_CMD_INIT1, PIT_CMD_PORT);
    outb(PIT_CH0_INIT2, PIT_CH0_PORT);
    outb(PIT_CH0_INIT3, PIT_CH0_PORT);
    enable_irq(PIT_IRQ);
    restore_flags(flags);
}


/*
    pit_rtc_handler
    Input: None
    Output: none
    Effects: handler for pit when it interrupts
*/
void pit_irq_handler() {
    int current_esp;
    int current_ebp;
    asm ("movl %%esp, %0;"
         "movl %%ebp, %1;"
         :"=r" (current_esp), "=r" (current_ebp)
    );
    
    esp_ebp_term[process_terminal][0] = current_esp;
    esp_ebp_term[process_terminal][1] = current_ebp;

    send_eoi(PIT_IRQ);
    scheduler();
}
