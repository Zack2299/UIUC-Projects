#define ASM 1


syscall_table:
    .long   0x0 /*syscalls are 1 indexed*/
    .long   halt
    .long   execute
    .long   read
    .long   write
    .long   open
    .long   close
    .long   getargs
    .long   vidmap


.globl syscall_handler
syscall_handler:
    /*check if call is valid*/
    cmpl $1, %eax
    jl   sys_error
    cmpl $8, %eax
    jg   sys_error
    
    /*push all registers*/
    pushl %ebp
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %esp
    pushfl

    /*push potential args*/
    pushl %edx
    pushl %ecx
    pushl %ebx

    /*call syscall using jumptable*/
    call *syscall_table(, %eax, 4)

    /*removing args*/
    addl $12, %esp

    /*pop all regs*/
valid:
    popfl
    popl %esp
    popl %edi
    popl %esi
    popl %ebx
    popl %edx
    popl %ecx
    popl %ebp
    iret 

sys_error:
    movl $-1, %eax
    iret

.globl halt_asm
halt_asm:
    pushl %ebp
    movl %esp, %ebp

    movl 16(%ebp), %eax     # get third arg (status)
    # movl %ebx, %eax         
    # movl %ebx, %esp   
    # movl 12(%ebp), %esp     # get second arg (esp)
    movl 8(%ebp), %ebx
    movl %ebx, %ebp       # get first arg (ebp)
    leave
    ret

.globl	schedule_asm
schedule_asm:
    pushl %ebp
    movl %esp, %ebp

    movl 12(%ebp), %esp     # get second arg (esp)
    movl 8(%ebp), %ebx
    movl %ebx, %ebp       # get first arg (ebp)
    # movl %ebx, %ebp

    leave
    ret

