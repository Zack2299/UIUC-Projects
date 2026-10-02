#include "idtinit.h"
#include "exception_handler.h"
#include "asmlinkage.h"

/*
 *idt_init
 *  DESCRIPTION: Initializes the idt. Connects it so that
 *  when each exception is raised (and when the keyboard and rtc
 *  raise interrupts) it will call the correct corresponding function.
 *  Input: a pointer to the idt
 *  Output: none
 * 
 */
extern void idt_init(idt_desc_t * the_idt) {
    int i;
    // set all 256 spots of the idt
    for (i = 0; i < NUM_VEC; i++) {
        the_idt[i].seg_selector = KERNEL_CS;
        the_idt[i].reserved4 = 0x00;
        the_idt[i].reserved2 = 0x1;
        the_idt[i].reserved1 = 0x1;
        the_idt[i].reserved0 = 0x0;
        the_idt[i].size = 0x1; // 1 means 32 bits
        the_idt[i].dpl = 0x0;
        
        // if one of the reserved exceptions or in the range 32-255
        if (i == 0x09 || i == 0x0F || i >= 32) {
            the_idt[i].present = 0x0;
            the_idt[i].reserved3 = 0x0; // interrupt
        } else { // 0-31 are exceptions
            the_idt[i].present = 0x1;
            the_idt[i].reserved3 = 0x1; // exception
        }

    }

    // set idt entries so that we can run corresponding exception/interrupt handlers
    SET_IDT_ENTRY(the_idt[0x00], divide_error);
    SET_IDT_ENTRY(the_idt[0x01], debug_exception);
    SET_IDT_ENTRY(the_idt[0x02], nmi_interrupt);
    SET_IDT_ENTRY(the_idt[0x03], breakpoint_exception);
    SET_IDT_ENTRY(the_idt[0x04], overflow_exception);
    SET_IDT_ENTRY(the_idt[0x05], bound_range_exceeded);
    SET_IDT_ENTRY(the_idt[0x06], invalid_opcode);
    SET_IDT_ENTRY(the_idt[0x07], device_not_available);
    SET_IDT_ENTRY(the_idt[0x08], double_fault);
    SET_IDT_ENTRY(the_idt[0x09], RESERVED09); /* 0x09 RESERVED BY INTEL */
    SET_IDT_ENTRY(the_idt[0x0A], invalid_tss);
    SET_IDT_ENTRY(the_idt[0x0B], segment_not_present);
    SET_IDT_ENTRY(the_idt[0x0C], stack_fault);
    SET_IDT_ENTRY(the_idt[0x0D], general_protection_exception);
    SET_IDT_ENTRY(the_idt[0x0E], page_fault);
    SET_IDT_ENTRY(the_idt[0x0F], RESERVED0F); /* 0x0F RESERVED BY INTEL */
    SET_IDT_ENTRY(the_idt[0x10], fpu_floating_point_error);
    SET_IDT_ENTRY(the_idt[0x11], alignment_check_exception);
    SET_IDT_ENTRY(the_idt[0x12], machine_check_exception);
    SET_IDT_ENTRY(the_idt[0x13], simd_floating_point_error);

    // 0x20 is the 1st spot (0) of the primary pic. PIT is attached here so make it present
    the_idt[0x20].present = 0x1;
    SET_IDT_ENTRY(the_idt[0x20], pit_interrupt);

    // 0x21 is the 2nd spot (1) of the primary pic. keyboard is attached here so make it present
    the_idt[0x21].present = 0x1;
    SET_IDT_ENTRY(the_idt[0x21], keyboard_interrupt);

    // 0x28 is the first spot (0) of the secondary pic, which is what rtc is attached to
    the_idt[0x28].present = 0x1;
    SET_IDT_ENTRY(the_idt[0x28], rtc_interrupt);

    the_idt[SYSCALLS].present = 0x1;
    the_idt[SYSCALLS].dpl = 0x3;
    SET_IDT_ENTRY(the_idt[SYSCALLS], syscall_handler);
}

