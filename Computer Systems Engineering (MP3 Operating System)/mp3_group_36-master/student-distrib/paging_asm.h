#pragma once
#include "paging.h"

#ifndef _PAGING_ASM_H_
#define _PAGING_ASM_H_

extern void load_page_directory(pde_t* page_directory_addr);

//extern void enable_paging();

extern void tlb_flush();

#endif
