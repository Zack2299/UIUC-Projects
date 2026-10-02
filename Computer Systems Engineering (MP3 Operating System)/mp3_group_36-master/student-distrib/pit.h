#pragma once
/*****************************
 * This file holds all Macros 
 * and functions used in pit.c
 * 
*******************************/
#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "sys_call.h"
#include "terminal.h"

#define PIT_IRQ         0
#define MAX_FREQ_PIT    1193280
#define DESIRED_FREQ    100
#define PIT_CMD_PORT    0x43
#define PIT_DATA_PORT   0x40

extern void pit_init(void);
extern void pit_handler(void);


#endif
