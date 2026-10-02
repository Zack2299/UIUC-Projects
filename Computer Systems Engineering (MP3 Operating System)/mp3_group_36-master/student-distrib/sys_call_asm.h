#pragma once
#ifndef _SYS_CALL_ASM_H
#define _SYS_CALL_ASM_H
#include "x86_desc.h"

#ifndef ASM

extern void syscall_handler();

extern void push_iret_context(int32_t entry_point);

extern void schedule_asm(uint32_t ebp, uint32_t esp);

#endif // ASM
#endif // _ASM_LINK
