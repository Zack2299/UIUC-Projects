#pragma once
#ifndef _SYS_CALL_STRUCT_H
#define _SYS_CALL_STRUCT_H

#include "types.h"

#define BUFFER_SIZE 128

/*Function pointer structs*/
typedef struct {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} funcPointers_t;

/* File Descriptor Table */
typedef struct {
    funcPointers_t* file_operations_table_ptr;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
} file_desc_t;

/*PCB struct*/
typedef struct {
    uint32_t pid;                       // cur proc == PID
    uint32_t parent_id;                 // as of cp3, make it -1?
    int32_t terminal_id;
    file_desc_t file_desc_table[8];
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t active;                    // as of cp3, its a flag that informs its on or off
    uint8_t args[128];
} pcb_t;

typedef struct{
    int process_id;
    int screen_x;
    int screen_y;
    char keyboard_buf[BUFFER_SIZE];
    int charCount;
    int enterFlag;
    int rtc_max;
    int fish;
    pcb_t* cur_process;
} process_info_t;
// process_info_t multi_process[3];
process_info_t* running_terminal;

#endif
