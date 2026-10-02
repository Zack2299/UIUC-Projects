/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

void tux_init(struct tty_struct* tty);
void tux_buttons(struct tty_struct* tty, int arg);
void tux_set_led(struct tty_struct* tty, int arg);

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

#define NUM_LEDS	4
#define LSB_MASK	0x01
# define CHANGE_ALL_LEDS		0x0F

unsigned char opcode;
unsigned char buttons = 0xF; // buttons all off to start (0xF)
unsigned char arrow_buttons = 0xF; // buttons all off to start (0xF)
//char restore_leds[6]; // save value of what leds were last set to for reset commands
int arg_restore = 0;
int set_led_flag = 0; // flag to handle spam input


/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{

    opcode = packet[0]; /* Avoid printk() sign extending the 8-bit */ /* values when printing them. */

	switch(opcode) {
		case MTCP_BIOC_EVENT:
			buttons = packet[1]; // c, b, a, start
    		arrow_buttons = packet[2]; // right, down, left, up
			break;
		case MTCP_RESET:
			tux_init(tty);
			tux_set_led(tty, arg_restore); // restore what leds were last set to
			break;
		case MTCP_ACK:
			set_led_flag = 0;
			break;
	}

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		tux_init(tty);
		return 0;
	case TUX_BUTTONS:
		if (arg == 0)
			return -EINVAL;
		tux_buttons(tty, arg);
		return 0;
	case TUX_SET_LED:
		if (set_led_flag == 0) { //"lock" to handle spam input
			set_led_flag = 1;
			tux_set_led(tty, arg);
		}
		return 0;

	// the next 3 will do the return -EINVAL b/c no break or return
	case TUX_LED_ACK:
	case TUX_LED_REQUEST:
	case TUX_READ_LED:
	default:
	    return -EINVAL;
    }
}

/*
 * tux_init
 *   DESCRIPTION: initializes the tux controller
 * 					to enable button interrupts
 * 					and allow it to display in user-mode
 *   INPUTS: tty struct for ldisc_put
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes the tux
 */
void tux_init(struct tty_struct* tty) {
	char buffer[2];
	set_led_flag = 0; // reset flag (unlock)
	buffer[0] = MTCP_BIOC_ON; // initialize to enable button interrupt-on-change
	buffer[1] = MTCP_LED_USR; // initialize LED display to be in user-mode
	tuxctl_ldisc_put(tty, buffer, 2);
}

/*
 * tux_buttons
 *   DESCRIPTION: 
 *   INPUTS: tty struct for ldisc_put, arg that is a pointer to a 32-bit integer
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies input from tux over to user space in order to have
 * 					our program communicate with the tux
 */
void tux_buttons(struct tty_struct* tty, int arg) {
	int i;
	char bit_mask = 0x10; // start at 0b10000 and then right shift 4 times in the for loop
	char all_buttons = 0x00; // want: right left down up c b a start
	// buttons - c, b, a, start
    // arrow_buttons - right, down, left, up

	// put in arrow buttons (0x08, 0x04, 0x02, and 0x01 are bit masks for the correct position for right, down, left, and up)
	all_buttons += 0x08 & arrow_buttons;
	all_buttons += (0x02 & arrow_buttons) << 1; // swap positions of
	all_buttons += (0x04 & arrow_buttons) >> 1; // left and down
	all_buttons += 0x01 & arrow_buttons;

	// put in buttons (no flipping positions so we can use a for loop)
	for (i = 3; i >= 0; i--) {
		all_buttons <<= 1;
		bit_mask >>= 1;
		all_buttons += (bit_mask & buttons) >> i;
	}
 
	//printk("%x\n", all_buttons); // <- for testing
	copy_to_user((int*) arg, &all_buttons, 1);
}

/*
 * tux_set_led
 *   DESCRIPTION: 
 *   INPUTS: tty struct for ldisc_put, arg with information of how to set the leds
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the leds of the tux controller
 */
void tux_set_led(struct tty_struct* tty, int arg) {
	char led_off = 0x00; // send this when we want the led to be off
	char buffer[6];
	char hex_convert[16] = {0xE7, 0x06, 0xCB, 0x8F, // 0, 1, 2, 3
							0x2E, 0xAD, 0xED, 0x86, // 4, 5, 6, 7
							0xEF, 0xAE, 0xEE, 0x6D, // 8, 9, A, B
							0xE1, 0x4F, 0xE9, 0xE8}; // C, D, E, F
	int bit_mask = 0x000F0000; // lower 4 bits of 3rd byte
	char leds_on = (arg & bit_mask) >> 16; // get lower 4-bits of 3rd byte for which leds are on and then shift all the way to the right (16)
	char points_on = (arg & (bit_mask << 8)) >> 24; // move bit mask to lower bits of 4th byte instead of the 3rd and then shift all the way to the right (24)
	int i;
	char num_to_disp[NUM_LEDS];

	arg_restore = arg; // store arg for reset to be able to go back to what the leds were last set to
	for (i = NUM_LEDS - 1; i >= 0; i--) { // loop 4 times
		bit_mask >>= 4; // shift the mask (0xF000, then 0x0F00, ... to get number for each led)
		num_to_disp[i] = hex_convert[(arg & bit_mask) >> (4 * i)]; // 4 * i to move over by 4 bits
	}

	buffer[0] = MTCP_LED_SET; // tell it to set the leds
	buffer[1] = CHANGE_ALL_LEDS; // tell it that we want to change all of them (set them to the correct value if they're
								// on or set them to NULL if they're off)

	for (i = 0; i < NUM_LEDS; i++) {
		if ((points_on >> i) & LSB_MASK) // check if current led's decimal point should be on
			num_to_disp[i] |= 0x10; // 0x10 is the bit for the decimal point

		buffer[i + 2] = num_to_disp[i]; // fill remaining 4 spots of buffer with led hex values
	}

	tuxctl_ldisc_put(tty, buffer, 2); // always put first 2 elements from buffer
	for (i = 0; i < NUM_LEDS; i++) {
		if ((leds_on >> i) & LSB_MASK)
			tuxctl_ldisc_put(tty, &buffer[i + 2], 1); // conditionally put leds based on whether they should be on or not (skip first 2 elements)
		else
			tuxctl_ldisc_put(tty, &led_off, 1); // NULL if led is off

	}
}
