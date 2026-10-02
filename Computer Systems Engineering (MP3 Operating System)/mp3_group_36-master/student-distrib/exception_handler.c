#include "exception_handler.h"

char * exception_text[20] = {
    "Division Error",
    "Debug Exception",
    "Non-maskable Interrupt",
    "Breakpoint Exception",
    "Overflow Exception",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "RESERVED BY INTEL",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Exception",
    "Page-Fault",
    "RESERVED BY INTEL",
    "FPU Floating Point Error",
    "Alignment Check Exception",
    "Machine-Check Exception",
    "SIMD Floating Point Exception",
};

extern void handle_exception(int num) {
    if (num < 0 || num > 19) {
        printf("Exception outside range 0-19");
        while(1);
    }

    printf(exception_text[num]);
    /* tell code excpetion happened */
    halt(num);
}

// extern void page_fault_func() {
//     printf("[Page-Fault CALLED BY FUNCTION]\n");

//     int terminal_num = current_terminal_num();
//     printf("Current process: %d\n", multi_process[terminal_num].cur_process->pid);
// }
