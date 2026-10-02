#pragma once
#ifndef _SYS_CALL_H
#define _SYS_CALL_H

#include "file_sys.h"
#include "sys_call_structs.h"
// #include "terminal.h"
#include "rtc.h"
#include "types.h"
//#include "paging_asm.h"

#define MAGIC_NUM_1     0x7f
#define MAGIC_NUM_2     0x45
#define MAGIC_NUM_3     0x4c
#define MAGIC_NUM_4     0x46
#define EIGHT_KB        8192
#define _128MB_PLUS_OFFSET  0x8048000
#define PROGRAM1        3
#define PROGRAM2        4
#define PROGRAM3        5
// int cur_process_num = -1;



/*
    Each variable will hold a pointer to all 4 of it's driver functions, 
    arguments of each driver function MUST be exactly the SAME as the struct.
    See funcPointers_t struct above for reference
*/
funcPointers_t rtc_fp;      //RTC
funcPointers_t file_fp;     //File
funcPointers_t dir_fp;      //Directory
funcPointers_t stdin_fp;    //Terminal read only
funcPointers_t stdout_fp;   //Terminal write only


// pcb_t pcb_array[10]; //I don't think you need this either but I'll leave it in

/*I don't believe you need this since it's already inside of PCB struct*/
// file_desc_t file_desc_table[8]; // eventually change to use pid one in functions

void schedule(void);

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);
extern void funcPointers_int(void);
int ret_neg_1();
extern pcb_t* get_pcb(int cur_process_num);
extern int get_pcb_idx();
extern int32_t halt_asm(uint32_t saved_ebp, uint32_t saved_esp, uint8_t status);
extern int get_scheduled_term();
extern int32_t process_free(int32_t pid);
extern int32_t process_allocate(void);
#endif
