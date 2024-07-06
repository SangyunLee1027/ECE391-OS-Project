#include "idt.h"
#include "syscall.h"

char* exception_string[] = {
    "Division Error", "Debug", "Non-maskable Interrupt", "Breakpoint", " Overflow", "BOUND Range Exceeded", "Invalid Opcode",
    "Device Not Available", "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
    "Stack-Segment Fault", "General Protection", "Page Fault", "(intel reserved, Do not use)", "x87 FPU Floating-Point Error",
    "Alignment Check", "Machine Check", "SIMD Floating-Point Exception"
};

// init_idt
//Description: it initializes the interrupt descriptor table using the function SET_IDT_ENTRY
//and handler written in interrupt_wrapper_asm.S.
//Input: None
//Output: None
//Effects: initialize the interrupt descriptor table for every interruption and exception
void init_idt(){

    int i = 0;

    for(i = 0; i < 20; ++i){
        if(i == 15){
            continue;
        }   // trap gate
        idt[i].dpl = 0; // kernel level privillege
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].size = 1;
        idt[i].present = 1;
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
    }

    SET_IDT_ENTRY(idt[0], division_error);
    SET_IDT_ENTRY(idt[1], debug);
    SET_IDT_ENTRY(idt[2], nmi_interrupt);
    SET_IDT_ENTRY(idt[3], breakpoint);
    SET_IDT_ENTRY(idt[4], overflow);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded);
    SET_IDT_ENTRY(idt[6], invalid_opcode);
    SET_IDT_ENTRY(idt[7], device_not_available);
    SET_IDT_ENTRY(idt[8], double_fault);
    SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], invalid_tss);
    SET_IDT_ENTRY(idt[11], segment_not_present);
    SET_IDT_ENTRY(idt[12], stack_segment_fault);
    SET_IDT_ENTRY(idt[13], general_protection);
    SET_IDT_ENTRY(idt[14], page_fault);
    // SET_IDT_ENTRY(idt[15], intel_reserved);  // reserved
    SET_IDT_ENTRY(idt[16], x87_FPU_floating_point_error);
    SET_IDT_ENTRY(idt[17], alignment_check);
    SET_IDT_ENTRY(idt[18], machine_check);
    SET_IDT_ENTRY(idt[19], SIMD_Floating_point_exception);


    //set idt for system call
    // trap gate
    idt[SYSTEM_CALL_ADDR].dpl = 3; // user level privillege
    idt[SYSTEM_CALL_ADDR].reserved0 = 0;
    idt[SYSTEM_CALL_ADDR].reserved1 = 1;
    idt[SYSTEM_CALL_ADDR].reserved2 = 1;
    idt[SYSTEM_CALL_ADDR].reserved3 = 1;
    idt[SYSTEM_CALL_ADDR].reserved4 = 0;
    idt[SYSTEM_CALL_ADDR].seg_selector = KERNEL_CS;
    idt[SYSTEM_CALL_ADDR].present = 1;
    idt[SYSTEM_CALL_ADDR].size = 1;

    SET_IDT_ENTRY(idt[SYSTEM_CALL_ADDR], syscall_handler);

    // set idt for rtc
    // trap gate
    idt[RTC_ADDR].dpl = 0;  // kernel level privillege
    idt[RTC_ADDR].reserved0 = 0;
    idt[RTC_ADDR].reserved1 = 1;
    idt[RTC_ADDR].reserved2 = 1;
    idt[RTC_ADDR].reserved3 = 1;
    idt[RTC_ADDR].reserved4 = 0;
    idt[RTC_ADDR].seg_selector = KERNEL_CS;
    idt[RTC_ADDR].present = 1;
    idt[RTC_ADDR].size = 1;

    SET_IDT_ENTRY(idt[RTC_ADDR], rtc_handler_helper); //PIC: 0x20, rtc: irq8


    // set idt for keyboard
    // trap gate
    idt[KEYBOARD_ADDR].dpl = 3; // user level privillege
    idt[KEYBOARD_ADDR].reserved0 = 0;
    idt[KEYBOARD_ADDR].reserved1 = 1;
    idt[KEYBOARD_ADDR].reserved2 = 1;
    idt[KEYBOARD_ADDR].reserved3 = 1;
    idt[KEYBOARD_ADDR].reserved4 = 0;
    idt[KEYBOARD_ADDR].seg_selector = KERNEL_CS;
    idt[KEYBOARD_ADDR].present = 1;
    idt[KEYBOARD_ADDR].size = 1;

    SET_IDT_ENTRY(idt[KEYBOARD_ADDR], keyboard_handler_helper); //PIC: 0x20, rtc: irq8

    idt[PIT_ADDR].dpl = 0; // kernel level privillege
    idt[PIT_ADDR].reserved0 = 0;
    idt[PIT_ADDR].reserved1 = 1;
    idt[PIT_ADDR].reserved2 = 1;
    idt[PIT_ADDR].reserved3 = 1;
    idt[PIT_ADDR].reserved4 = 0;
    idt[PIT_ADDR].seg_selector = KERNEL_CS;
    idt[PIT_ADDR].present = 1;
    idt[PIT_ADDR].size = 1;

    SET_IDT_ENTRY(idt[PIT_ADDR], pit_handler_helper); 
}
//static int r_eax, rfl;
// exception_handler
// Description: this function is handling all exceptions in check point 1 by printing which exception occured, and 
// pause the system using while(1)
// Input: idx - index which tells what exception occured
// Output: none
// Effect: prints the name of exception and pause the system
void exception_handler(int idx){

    if(idx < 0 || idx > 19){ // check if idx in range
        return;
    }


}



// systemcall_checker
// Description: this function is handling system_call interruption. For this checkpoint, we just need to see the systemcall interruption is working
// so just prints the string that shows it is working.
// Input: None
// Output: None
// Effect: prints the "system call"
void systemcall_checker(){
    printf("system call");

    while(1);
}
