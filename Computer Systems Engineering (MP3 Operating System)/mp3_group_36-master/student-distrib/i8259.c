/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    /* Local variables */
    int i;
    /* Set PICs to cascade mode */
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    /* 
    Write interrupt vectors for PICs
        Master: x20 ~ x27
        Slave: x28 ~ x2F
    */
    outb(ICW2_MASTER, MASTER_8259_DATA_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT);

    /* Specify IR pin used in master/slave */
    outb(ICW3_MASTER, MASTER_8259_DATA_PORT);
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);

    /* Specify 8086 protocols (normal EOL, etc) */
    outb(ICW4, MASTER_8259_DATA_PORT);
    outb(ICW4, SLAVE_8259_DATA_PORT);

    /* Mask all IRQs to prevent infinite booting */
    for (i = 0; i < 15; i++) {
        disable_irq(i);
    }
    enable_irq(2);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    /* Local variables */
    uint16_t port = 0;
    uint8_t data = 0;
    
    /* Check whether first PIC or second PIC */
    if (irq_num < 8) {
        port = MASTER_8259_DATA_PORT; 
    }
    else {
        port = SLAVE_8259_DATA_PORT;
        irq_num -= PORT_OFFSET;
    }

    /* 
    Grab PIC value and set to '0' to mask 
        EX:
            data = 1011 & ~(1 < IR1) -> data = 1001
    */
    data = inb(port) & ~(1 << irq_num);
    outb(data, port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    /* Local variables */
    uint16_t port = 0;
    uint8_t data = 0;
    
    /* Check whether first PIC or second PIC */
    if (irq_num < 8) {
        port = MASTER_8259_DATA_PORT; 
    }
    else {
        port = SLAVE_8259_DATA_PORT;
        irq_num -= PORT_OFFSET;
    }

    /* 
    Grab PIC value and set to '1' to mask 
        EX:
            data = 1001 | 1 < IR1 -> data = 1011
    */
    data = inb(port) | (1 << irq_num);
    outb(data, port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    /* IRQ came from slave */
    if (irq_num > 7) {
        outb(EOI, SLAVE_8259_PORT);
    }

    /* IRQ came from master */
    outb(EOI, MASTER_8259_PORT);
}
