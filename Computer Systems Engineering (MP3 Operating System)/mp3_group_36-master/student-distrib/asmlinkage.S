#define ASM 1


#define LINK(name, func)   \
# .globl name                 ;\
# name:                       ;\
#     pushal                  ;\
#     pushfl                  ;\
#     call func               ;\
#     popfl                   ;\
#     popal                   ;\
#     iret                    

#LINK(page_fault, page_fault_func);

#define INTR_LINK(name, num)   \
.globl name                 ;\
name:                       ;\
    pushal                  ;\
    pushfl                  ;\
    pushl $num               ;\
    call handle_exception               ;\
    addl $4,%esp           ;\
    popfl                   ;\
    popal                   ;\
    iret          

INTR_LINK(divide_error,     0x00);
INTR_LINK(debug_exception,  0x01);
INTR_LINK(nmi_interrupt,    0x02);
INTR_LINK(breakpoint_exception, 0x03);
INTR_LINK(overflow_exception, 0x04);
INTR_LINK(bound_range_exceeded, 0x05);
INTR_LINK(invalid_opcode, 0x06);
INTR_LINK(device_not_available, 0x07);
INTR_LINK(double_fault, 0x08);
INTR_LINK(RESERVED09, 0x09); /* 0x09 RESERVED BY INTEL */
INTR_LINK(invalid_tss, 0x0A);
INTR_LINK(segment_not_present, 0x0B);
INTR_LINK(stack_fault, 0x0C);
INTR_LINK(general_protection_exception, 0x0D);
INTR_LINK(page_fault, 0x0E);
INTR_LINK(RESERVED0F, 0x0F); /* 0x0F reserved by intel */
INTR_LINK(fpu_floating_point_error, 0x10);
INTR_LINK(alignment_check_exception, 0x11);
INTR_LINK(machine_check_exception, 0x12);
INTR_LINK(simd_floating_point_error, 0x13);


#define KEYBOARD_LINK(name)   \
.globl name                 ;\
name:                       ;\
    pushal                  ;\
    pushfl                  ;\
    call keyboard_handler               ;\
    popfl                   ;\
    popal                   ;\
    iret          

KEYBOARD_LINK(keyboard_interrupt);

#define RTC_LINK(name)   \
.globl name                 ;\
name:                       ;\
    pushal                  ;\
    pushfl                  ;\
    call rtc_handler               ;\
    popfl                   ;\
    popal                   ;\
    iret          

RTC_LINK(rtc_interrupt);

#define PIT_LINK(name)   \
.globl name                 ;\
name:                       ;\
    pushal                  ;\
    pushfl                  ;\
    call pit_handler               ;\
    popfl                   ;\
    popal                   ;\
    iret          

PIT_LINK(pit_interrupt);
