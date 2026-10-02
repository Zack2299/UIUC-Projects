#pragma once
#ifndef _FILE_SYS_H
#define _FILE_SYS_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "multiboot.h"
#include "sys_call.h"

#define SIZE_OF_BOOT_BLOCK_RESERVED     52
#define SIZE_OF_BOOT_BLOCK_DIR_ENTRIES  64
#define SIZE_OF_FILE_NAME               32
#define SIZE_OF_DIR_ENTRIES_RESERVED    24
#define SIZE_OF_DATA_BLOCK_NUM_ARRAY    1023
#define SIZE_OF_DATA_BLOCK              1024


#define NUMBER_OF_DIR_ENTIRES           63
#define FD_START                        0
#define FD_END                          7
#define FOUR_KB                         4096



#define MAX_FILE_NAME_SIZE  32



// extern void file_sys_init(((module_t*)mbi->mods_addr)->mods_start);

/* Directory Entries inside of Boot Block */
typedef struct {
    uint8_t file_name[SIZE_OF_FILE_NAME];
    uint32_t file_type;
    uint32_t inode;
    uint8_t reserved[SIZE_OF_DIR_ENTRIES_RESERVED];
} dir_entry_t;

/* Boot Block Entries */
typedef struct {
    uint32_t num_dir_entries; 
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[SIZE_OF_BOOT_BLOCK_RESERVED];
    dir_entry_t dir_entries[NUMBER_OF_DIR_ENTIRES];
} boot_block_t;


/* Inode Entries */
typedef struct {
    uint32_t len_in_B;
    uint32_t data_block_num[SIZE_OF_DATA_BLOCK_NUM_ARRAY];
} inode_t;

/* Data Blocks */
typedef struct {      
    uint8_t data_block[FOUR_KB];
} data_block_t;

// boot_block_t* blocks_start;

boot_block_t* blocks_start;
inode_t* inode_start;
data_block_t* data_block_start;


/* ----- file_sys.c ----- */
/* take in ptr to traverse blocks */
extern int file_sys_init(module_t* mbi);

// extern int file_sys_open(module_t* mbi);
extern int file_sys_open(const uint8_t* filename);

extern int file_sys_close(int32_t fd);

extern int file_sys_read(int32_t fd, void* buf, int32_t nbytes);

extern int file_sys_write(int32_t fd, const void* buf, int32_t nbytes);



/* ----- file_driver.c ----- */
extern int32_t read_directory(int32_t fd, void* buf, int32_t nbytes);

extern int32_t open_directory(const uint8_t* filename);

extern int32_t close_directory(int32_t fd);

extern int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes);

extern uint32_t read_dentry_by_name(const uint8_t* file_name, dir_entry_t* dir_entry);

extern uint32_t read_dentry_by_index(const uint32_t index, dir_entry_t* dir_entry);

uint32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* helper functions to traverse the file system */
extern dir_entry_t* traverse_dentry_by_name(const uint8_t* file_name);
extern dir_entry_t* traverse_dentry_by_index(const uint32_t index);



// uint32_t read_dentry_by_name(const uint8_t* file_name, dir_entry_t* dir_entry);
// uint32_t read_dentry_by_index(const uint32_t index, dir_entry_t* dir_entry);
// uint32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// dir_entry_t* traverse_dentry_by_name(const uint8_t* file_name);
// dir_entry_t* traverse_dentry_by_index(const uint32_t index);



#endif
