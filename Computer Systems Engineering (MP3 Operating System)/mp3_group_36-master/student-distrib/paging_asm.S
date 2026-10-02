#define ASM 1

.text
.globl load_page_directory
load_page_directory:
/*Stack set up*/
pushl %ebp
movl %esp, %ebp
/*Grabbing starting address of page directory table and putting it in cr3*/
movl 8(%esp), %eax
movl %eax, %cr3
/*Enables 4MiB pages*/
movl %cr4, %eax
orl $0x00000010, %eax
movl %eax, %cr4
/*Enables paging*/
movl %cr0, %eax
orl $0x80000000, %eax
movl %eax, %cr0
/*stack tear down*/
leave
ret

.text
.globl tlb_flush
tlb_flush:
pushl %ebp
movl %esp, %ebp

movl %cr3,%eax
movl %eax,%cr3

movl %ebp, %esp
popl %ebp
ret
