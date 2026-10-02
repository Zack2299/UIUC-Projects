
#include "pit.h"
/* 
*   pit_init()
*   Description: Initialized the pit to schedule terminals 
*   Input: None
*   Output: None
*   Return Val: None
*   Effect: Interrupts process every 10 ms
*/
void pit_init(void){
    /*
        The max default frequency is 1.19 MHz (1193180 Hz) and we want an interupt every 10ms
        which is 100 interputs per second so we need to find the number to solve this equation:
        1193180 / x = 100;
        This info was found at http://www.osdever.net/bkerndev/Docs/pit.htm
    */   
    int divisor = MAX_FREQ_PIT / DESIRED_FREQ;       /* Calculate our divisor */
    outb(0x37, PIT_CMD_PORT);             /* Set our command byte 0x36 */
    outb(divisor & 0xFF, PIT_DATA_PORT);   /* Set low byte of divisor */
    outb(divisor >> 8, PIT_DATA_PORT);     /* Set high byte of divisor */

    enable_irq(PIT_IRQ);
}

/* 
*   pit_handler()
*   Description: When a pit interupt occurs this function gets called 
*   Input: None 
*   Output: None
*   Return Val: 
*   Effect: Remaps the memory to the next scheduled terminal 
*/void pit_handler(void){
    /* initialize */
    
    send_eoi(PIT_IRQ);
    video_mem_remap(((running_terminal->process_id)+1)%3);
    schedule();
}


