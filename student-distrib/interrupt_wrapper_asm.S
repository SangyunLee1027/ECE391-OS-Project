#define ASM     1


#define HANDLE_EXCEPTION(name, index) \
    .global name                     ;\
    name:                            ;\
        pushal                       ;\
        pushfl                       ;\
        pushl $index                 ;\
        call exception_handler       ;\
        addl $4, %esp                ;\
        popfl                        ;\
        popal                        ;\
        iret                         ;\

# save all registers
# save the flag
# insert the input
# call exception handler function
# pop the input
# pop the flag
# pop all registers
# interrupt return

#define HANDLE_EXCEPTION_(name, func) \
    .global name                     ;\
    name:                            ;\
        pushfl;\
        pushl %eax;\
        pushl   %esi;\
        pushl   %edi;\
        pushl   %edx;\
        pushl   %ecx ;\
        pushl   %ebx;\
        call func                    ;\
        popl    %ebx;\
        popl    %ecx;\
        popl    %edx;\
        popl    %edi;\
        popl    %esi;\
        popl    %eax;\
        popfl;\
        iret                         ;\

# save all registers
# save the flag
# call function
# pop the flag
# pop all registers
# interrupt return

HANDLE_EXCEPTION(division_error, 0x0);
HANDLE_EXCEPTION(debug, 0x1);
HANDLE_EXCEPTION(nmi_interrupt, 0x2);
HANDLE_EXCEPTION(breakpoint, 0x3);
HANDLE_EXCEPTION(overflow, 0x4);
HANDLE_EXCEPTION(bound_range_exceeded, 0x5);
HANDLE_EXCEPTION(invalid_opcode, 0x6);
HANDLE_EXCEPTION(device_not_available, 0x7);
HANDLE_EXCEPTION(double_fault, 0x8);
HANDLE_EXCEPTION(coprocessor_segment_overrun, 0x9);
HANDLE_EXCEPTION(invalid_tss, 0xA);
HANDLE_EXCEPTION(segment_not_present, 0xB);
HANDLE_EXCEPTION(stack_segment_fault, 0xC);
HANDLE_EXCEPTION(general_protection, 0xD);
HANDLE_EXCEPTION(page_fault, 0xE);
HANDLE_EXCEPTION(intel_reserved, 0xF);
HANDLE_EXCEPTION(x87_FPU_floating_point_error, 0x10);
HANDLE_EXCEPTION(alignment_check, 0x11);
HANDLE_EXCEPTION(machine_check, 0x12);
HANDLE_EXCEPTION(SIMD_Floating_point_exception, 0x13);

HANDLE_EXCEPTION_(systemcall_handler, systemcall_checker);
HANDLE_EXCEPTION_(rtc_handler_helper, rtc_irq_handler);
HANDLE_EXCEPTION_(keyboard_handler_helper, keyboard_handler);
HANDLE_EXCEPTION_(pit_handler_helper, pit_irq_handler);
