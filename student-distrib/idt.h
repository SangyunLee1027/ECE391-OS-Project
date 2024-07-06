#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "interrupt_wrapper.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "pit.h"

#ifndef ASM

#define SYSTEM_CALL_ADDR    0x80 
#define PIC_ADDR            0x20
#define RTC_ADDR            0x28 // PIC_ADDR + 8
#define KEYBOARD_ADDR       PIC_ADDR+KEYBOARD_IRQ
#define PIT_ADDR            PIC_ADDR+PIT_IRQ


extern void init_idt();
extern void exception_handler(int idx);
extern void systemcall_checker();

#endif
#endif
