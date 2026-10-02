#pragma once
#include "x86_desc.h"
#include "sys_call_asm.h"
#define SYSCALLS 0x80

extern void idt_init(idt_desc_t * the_idt);
