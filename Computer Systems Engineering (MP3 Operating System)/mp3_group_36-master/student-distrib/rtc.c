#include "rtc.h"
// int maxCount = MAX_FREQ;
// int rtcCounter = MAX_FREQ;
volatile int counter_max_reached = 0;

/* virtualization arrays */
volatile int rtc_counter[3] = {0, 0, 0};
volatile int rtc_counter_limit[3] = {0, 0, 0};
volatile int rtc_flag[3] = {1, 1, 1};
volatile int rtc_on[3] = {0, 0, 0};

// volatile int allow_counter = 0;
#define MAX_RATE 16
#define MIN_RATE 2

/*This psudeo code was provided by "https://wiki.osdev.org/RTC"*/
/*disable_ints();			// disable interrupts
outportb(0x70, 0x8B);		// select register B, and disable NMI
char prev=inportb(0x71);	// read the current value of register B
outportb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
outportb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
enable_ints();*/

/*
*   rtc_init()
*   Description: Initalizes the RTC 
*   Input: None
*   Output: None
*   Return Val: None
*   Effect: Connects RTC to IRQ 8 on the pic and sends periodic interupts using default 1024 Hz rate
*/
void rtc_init(void){
    uint32_t flags;
    cli_and_save(flags);
    /*select register B, and disable NMI*/
    outb((REG_B_IDX | NMI_MASK), RTC_IDX_PORT);
    /*read the current value of register B*/
    char prev = inb(RTC_CMOS_DATA);
    /*set the index again*/
    outb((REG_B_IDX | NMI_MASK), RTC_IDX_PORT);
    /*write the previous value ORed with 0x40. This turns on bit 6 of register B, enabling periodic interupts*/
    outb((prev | SIX_BIT_ON), RTC_CMOS_DATA);
    
    /*connects to primary 7*/
    enable_irq(RTC_IRQ);
    restore_flags(flags);
}


/*This psudeo code was provided by "https://wiki.osdev.org/RTC"*/
/*
outportb(0x70, 0x0C);	// select register C
inportb(0x71);		// just throw away contents
*/

/*
*   rtc_handler()
*   Description: Handles RTC interupts  
*   Input: None
*   Output: None
*   Return Val: None
*   Effect: Decrements the RTC counter and when it reaches 0 calls test_interputs function
*/
void rtc_handler(void){

    /* initialize */
    int i;
    
    process_info_t* allTerminals = ret_multi_process();
    /* run through all counters */
    for (i = 0; i < 3; i++) {
        /* current i is not on */
        if (allTerminals[i].rtc_max == 0) {
            continue;
        }
        /* increment counter */
        rtc_counter[i]++;
    }

    /*select register C*/
    outb(REG_C_IDX, RTC_IDX_PORT);

    /*just throw away contents*/
    inb(RTC_CMOS_DATA);
    /*send eoi signal*/
    send_eoi(RTC_IRQ);
}

/*This psudeo code was provided by "https://wiki.osdev.org/RTC"*/
/*rate &= 0x0F;			// rate must be above 2 and not over 15
disable_ints();
outportb(0x70, 0x8A);		// set index to register A, disable NMI
char prev=inportb(0x71);	// get initial value of register A
outportb(0x70, 0x8A);		// reset index to A
outportb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
enable_ints();*/


// frequency to 2HZ, freq = 32768 >> (rate -1 ) also from osdev.org
/* 
*   rtc_open()
*   Description: Opens and resets rtc freq  
*   Input: None
*   Output: None
*   Return Val: None
*   Effect: Resets the frequency to 2Hz
*/
int32_t rtc_open(const uint8_t* filename){
    /* set running terminal's rtc on flag to on */
    running_terminal->rtc_max = 1;  

    return 0;
}
/* 
*   rtc_close()
*   Description: Closes RTC
*   Input: None
*   Output: None
*   Return Val: None
*   Effect: Nothing
*/
int32_t rtc_close(int32_t fd){
    running_terminal->rtc_max = 0;
    rtc_counter[running_terminal->process_id] = 0;

    return 0;
}

/* 
*   rtc_write(int hz)
*   Description: Writes rtc freqency  
*   Input: int frequency in hertz
*   Output: # of bytes writen on a sucess, -1 on failure
*   Return Val: sizeof the value written
*   Effect: Wrtites a calculates the virtual frequency for the processes to run
*/

/*if input frequency is invalid (not multiple of 2) return -1*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    uint32_t flags,rate;
    cli_and_save(flags);
    int32_t hz;
    /*buf holds freq we want so we check for valid args*/
    if(buf == NULL)
        return -1;
    hz = *((int*) buf);
    rate = FREQRESET;
    // checks if hz is power of 2
    int temphz = hz;
    if((hz & (hz - 1)) != 0 || hz == 0) return -1;
    while(temphz > MIN_RATE){ // calculates the rate rate to set the hz
        temphz = temphz/MIN_RATE;
        rate--;
    }

    /* set limit for the rtc counter */

    running_terminal->rtc_max = MAX_FREQ / ((hz<<=1) + 1);
    restore_flags(flags);
 
    return 0; // return the # of bytes writen
}
/* 
*   rtc_read()
*   Description: Waits for interupt to finish before returning 
*   Input: 
*   Output: None
*   Return Val: 0
*   Effect: None
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    /* initialize */    
    sti();
    /* loop while rtc flag is high */
    while (rtc_counter[running_terminal->process_id] <= running_terminal->rtc_max) {
        /* do nothing */
    }
    rtc_counter[running_terminal->process_id] = 0;

    return 0;
}
