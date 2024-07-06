
#pragma once

#include "idt.h"

#ifndef ASM

extern void division_error(void);
extern void debug(void);
extern void nmi_interrupt(void);
extern void breakpoint(void);
extern void overflow( void);
extern void bound_range_exceeded( void);
extern void invalid_opcode( void);
extern void device_not_available( void);
extern void double_fault( void);
extern void coprocessor_segment_overrun( void);
extern void invalid_tss( void);
extern void segment_not_present( void);
extern void stack_segment_fault(void);
extern void general_protection(void);
extern void page_fault(void);
extern void intel_reserved(void);
extern void x87_FPU_floating_point_error(void);
extern void alignment_check(void);
extern void machine_check(void);
extern void SIMD_Floating_point_exception(void);
extern void systemcall_handler(void);
extern void rtc_handler_helper(void);
extern void keyboard_handler_helper(void);
extern void pit_handler_helper(void);

#endif
