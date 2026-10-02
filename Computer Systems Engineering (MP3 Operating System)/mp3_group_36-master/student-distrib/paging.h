#pragma once
#ifndef _PAGING_H
#define _PAGING_H

// #ifndef ASM

#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "terminal.h"

/* 
    01 | 00 0000 0000 
    ^         ^ lower 10 bits
    upper 2 bits
*/
#define KERNEL_PAGE_OFFSET_UPPER    0x01
#define KERNEL_PAGE_OFFSET_LOWER    0x000
#define PAGE_TABLE_VIDEO_MEM_OFFSET 0xB8
#define KERNEL_PAGE_ADDR            0x0400000 >> 22
#define VIDEO                       0xB8000

#define EIGHT_MB                    0x800000
#define FOUR_MB                     0x400000

#define _128MB_SHIFTED_R_22         0x08000000 >> 22
#define _136MB_SHIFTED_R_22         0x8800000 >> 22
#define _128MB                      0x8000000
#define _4KB                        4096


/* Page Directory Entries */
typedef union pde {
    uint32_t page_directory;
        struct {
            uint32_t p                       : 1;    /* 0 = not in mem/generates page-fault exception; 1 = in phys mem */
            uint32_t r_w                     : 1;    /* 0 = READ; 1 = READ/WRITE */
            uint32_t u_s                     : 1;    /* 0 = supervisor; 1 = user-level */
            uint32_t pwt                     : 1;    /* 0 = write-back caching enabled; 1 = write-through caching enabled */
            uint32_t pcd                     : 1;    /* 0 = can be cached; 1 = cannot be cached */
            uint32_t a                       : 1;    /* 0 = */
            uint32_t avl                     : 1;
            uint32_t ps                      : 1;
            uint32_t avail                   : 4;
            uint32_t page_table_base_addr   : 20;
        } page_dir_4kb __attribute__((packed));

        struct {
            uint32_t p                       : 1;    /* 0 = not in mem/generates page-fault exception; 1 = in phys mem */
            uint32_t r_w                     : 1;    /* 0 = READ; 1 = READ/WRITE */
            uint32_t u_s                     : 1;    /* 0 = supervisor; 1 = user-level */
            uint32_t pwt                     : 1;    /* 0 = write-back caching enabled; 1 = write-through caching enabled */
            uint32_t pcd                     : 1;    /* 0 = can be cached; 1 = cannot be cached */
            uint32_t a                       : 1;    /* 0 = */
            uint32_t d                       : 1;   
            uint32_t ps                      : 1;
            uint32_t g                       : 1;
            uint32_t avl                     : 3;
            uint32_t pat                     : 1;
            uint32_t bits39_to_32            : 8;
            uint32_t rsvd                    : 1;
            uint32_t page_table_base_addr   : 10;
        } page_dir_4mb __attribute__((packed));
} pde_t;

pde_t page_directory[1024] __attribute__((aligned(4096)));

/* Page Table Entries */
typedef union pte {
    uint32_t page_table;
        struct {
            uint32_t p               : 1;
            uint32_t r_w             : 1;
            uint32_t u_s             : 1;
            uint32_t pwt             : 1;
            uint32_t pcd             : 1;
            uint32_t a               : 1;
            uint32_t d               : 1;
            uint32_t pat             : 1;
            uint32_t g               : 1;
            uint32_t avail           : 3;
            uint32_t page_base_addr : 20;
        } page_table_4kb __attribute__ ((packed));
} pte_t;

pte_t page_table[1024] __attribute__((aligned(4096)));
pte_t vidmap_page_table[1024] __attribute__((aligned(4096)));

/* checkpoint 1 */
extern void paging_init();

/* checkpoint 3 */
extern void paging_set_up(uint32_t offset);
extern void paging_tear_down(uint32_t offset);
extern void user_vidmem_setup();
extern void user_vidmem_teardown();
extern void terminal_setup(int old_terminal, int new_terminal); 
extern void video_mem_remap(int nextTerminal);
extern void fish_remap(int nextTerminal);

#endif
