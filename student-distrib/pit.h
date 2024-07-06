#ifndef _PIT_H
#define _PIT_H

 #include "types.h"
 #include "i8259.h"
 #include "lib.h"

#ifndef ASM

#define PIT_IRQ         0x00

#define PIT_CH0_PORT    0x40 // Linked to IRQ0
#define PIT_CH1_PORT    0x41 // No longer used
#define PIT_CH2_PORT    0x42 // Linked to computer speaker
#define PIT_CMD_PORT    0x43

#define PIT_CMD_INIT1   0x36  //00 11 011 0 set Channel 0 to mode 3 (Square wave)
#define PIT_CH0_INIT2   0x38 //9C
#define PIT_CH0_INIT3   0x53 //2E


void pit_init();

void pit_irq_handler();

#endif
#endif

