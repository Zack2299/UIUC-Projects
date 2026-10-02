#pragma once
/***********************************************************
 * This file holds all the Macros used by the .c file
 * as well as functions
 * 
*************************************************************/
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"
#include "i8259.h"
#include "types.h"
#include "terminal.h"
// #include "sys_call.c"

// To follow standard set by https://wiki.osdev.org/Interrupts
#define KEYBOARD_IRQ 1

//ports for the keyboard also from above resource
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CMD_PORT 0x64

#define RELEASED 0x80
#define MAX_PRINTABLE 0x3E
#define BACKSPACE_CODE 0x08
#define NEWLINE_CHAR 0x0A

#define RIGHT_SHIFT_PRESS 0x36
#define LEFT_SHIFT_PRESS  0x2A
#define CAPS_LOCK_PRESS   0x3A
#define F1_PRESSED        0x3B
#define F2_PRESSED        0x3C
#define F3_PRESSED        0x3D
#define CTRL_PRESS        0x1D
#define ALT_PRESS         0x38
#define L_PRESS           0x26
#define ENTER_PRESS       0x1C
#define BACKSPACE_PRESS   0x0E
#define TAB_PRESS         0x0F
#define SPACE_PRESS       0x39

#define NUMS_LOWER_LIMIT 0x02
#define NUMS_UPPER_LIMIT 0x0D
#define FIRST_ROW_LOWER 0x10
#define FIRST_ROW_UPPER 0x1B
#define SECOND_ROW_LOWER 0x1E
#define SECOND_ROW_UPPER 0x29
#define THIRD_ROW_LOWER 0x2B
#define THIRD_ROW_UPPER 0x35
#define SHIFTABLE       2

extern void keyboard_init(void);

extern void keyboard_handler(void);

extern int special_key_pressed(uint8_t scanCode);
#endif /* _KEYBOARD_H */

