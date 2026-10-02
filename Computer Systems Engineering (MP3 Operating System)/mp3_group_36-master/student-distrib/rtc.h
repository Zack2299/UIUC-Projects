#pragma once
/*****************************
 * This file holds all Macros 
 * and functions used in rtc.c
 * 
*******************************/
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "sys_call_structs.h"


#ifndef _RTC_H
#define _RTC_H

#define RTC_IDX_PORT     0x70 // used to sepcify register number and to disable NMI -from: "https://wiki.osdev.org/RTC"
#define RTC_CMOS_DATA    0x71
#define RTC_IRQ          8

#define REG_A_IDX        0x0A
#define REG_B_IDX        0x0B
#define REG_C_IDX        0x0C
#define MAX_FREQ         1024
#define NMI_MASK         0x80
#define SIX_BIT_ON       0x40

#define BOTTOMFOURBITS   0xF0
#define FREQRESET        0x0F
/*initalizes the RTC and connects to pic*/
extern void rtc_init(void);
/*handles rtc interupts*/
extern void rtc_handler(void);

// rtc read, write, & close 
extern int32_t rtc_open(const uint8_t* filename);

extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

extern int32_t rtc_close(int32_t fd);


#endif /* _RTC_H */
