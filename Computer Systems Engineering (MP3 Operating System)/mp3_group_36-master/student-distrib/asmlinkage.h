#pragma once

#ifndef _ASM_LINK
#define _ASM_LINK

#ifndef ASM

extern void divide_error();
extern void debug_exception();
extern void nmi_interrupt();
extern void breakpoint_exception();
extern void overflow_exception();
extern void bound_range_exceeded();
extern void invalid_opcode();
extern void device_not_available();
extern void double_fault();

extern void RESERVED09();
/* RESERVED BY INTEL */

extern void invalid_tss();
extern void segment_not_present();
extern void stack_fault();
extern void general_protection_exception();
extern void page_fault();

extern void RESERVED0F();
/* RESERVED BY INTEL */

extern void fpu_floating_point_error();
extern void alignment_check_exception();
extern void machine_check_exception();
extern void simd_floating_point_error();


extern void keyboard_interrupt();
extern void rtc_interrupt();
extern void syscall_interrupt();
extern void pit_interrupt();


#endif // ASM
#endif // _ASM_LINK
