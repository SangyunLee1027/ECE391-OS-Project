/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* 
    i8259_init
    Description: Initialize the 8259 PIC 
    Input: None
    Output: None
    Effect: Initialize the 8259 PIC
*/
void i8259_init(void) {

    int i;
    // No interupts
    master_mask = 0;
    slave_mask = 0;

    // Start init
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    // Define Interrupt Vector offsets
    // Master starts at 0x20, Slave at 0x28
    outb(ICW2_MASTER, MASTER_8259_DATA_PORT); 
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT); 

    // Define slave at Master's IRQ2
    outb(ICW3_MASTER, MASTER_8259_DATA_PORT);
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);

    // Complete init with 8086/8088 mode enabled
    outb(ICW4, MASTER_8259_DATA_PORT);
    outb(ICW4, SLAVE_8259_DATA_PORT);

    for(i = 0; i < 16; ++i){ // disable all IRQs: 0 - 15
        disable_irq(i);
    }

    enable_irq(SLAVE_IRQ);
}


/* 
    enable_irq
    Description: enable the specified IRQ on PIC
    Input: irq_num - index of irq
    Output: None
    Effect: Enable (unmask) the specified IRQ 
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t data;

    if(irq_num > 15){   // check irq is in range
        return;
    }

    if (irq_num < 8) { // irq is from master
        port = MASTER_8259_DATA_PORT;
        master_mask &= ~(1 << irq_num); // enable masking
    } else { // irq is from slave
        port = SLAVE_8259_DATA_PORT;
        irq_num -= 8;               // Set irq_num to irq relative to slave
        slave_mask &= ~(1 << irq_num);  // enable masking
    }
    
    data = inb(port) & ~(1 << irq_num); // read the current mask value
    outb(data, port);
}


/* 
    disable_irq
    Description: disable the specified IRQ on PIC
    Input: irq_num - index of irq
    Output: None
    Effect: Disable (mask) the specified IRQ
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t data;

    if(irq_num > 15){   // check irq is in range
        return;
    }

    if (irq_num < 8) { // irq is from master
        port = MASTER_8259_DATA_PORT;
        master_mask |= 1 << irq_num;    // disable masking
    } else { // irq is from slave
        port = SLAVE_8259_DATA_PORT;
        irq_num -= 8;           // Set irq_num to irq relative to slave
        slave_mask |= 1 << irq_num;     // disable masking
    }

    data = inb(port) | (1 << irq_num);  // read the current mask value
    outb(data, port);
}



/* 
    send_eoi
    Description: Send end-of-interrupt signal for the specified IRQ. If secondary PIC is ended, then you have to send the end of interrupt signal for 
    IRQ on Master PIC connected to secondary PIC which IRQ2. 
    Input: irq_num - index of irq
    Output: None
    Effect: Send end-of-interrupt signal for the specified IRQ, so stop the interrupt by specific irq.
 */
void send_eoi(uint32_t irq_num) {

    if(irq_num > 15){   // check irq is in range
        return;
    }


    if (irq_num >= 8){ // Send EOI to slave too if irq is from slave
        outb(EOI | (irq_num-8), SLAVE_8259_PORT);
        outb(EOI | 0x2, MASTER_8259_PORT);
    }   else {
        outb(EOI | irq_num, MASTER_8259_PORT);
    }
    
}
