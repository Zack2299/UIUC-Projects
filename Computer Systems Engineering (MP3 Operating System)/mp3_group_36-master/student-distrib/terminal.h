/***********************************************************
 * This file holds all the Macros used by the .c file
 * as well as it's functions. Device driver for the terminal
 * 
*************************************************************/
#pragma once
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "types.h"
#include "sys_call.h"
#include "paging.h"
#include "sys_call_structs.h"


extern void getc(char c, int index);
extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_close(int32_t fd);
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
extern void switch_terminal(int terminal_id);
extern int current_terminal_num();
extern void terminal_init();

// extern int scheduled_terminal_num();


extern process_info_t* ret_running_term();
extern process_info_t* ret_multi_process();

#endif 
