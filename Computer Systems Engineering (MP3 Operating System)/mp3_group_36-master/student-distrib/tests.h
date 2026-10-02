#ifndef TESTS_H
#define TESTS_H

#define BEFORE_START_VID_MEM        0xB7FFF
#define START_VID_MEM               0xB8000
#define END_VID_MEM                 0xB8FFF
#define AFTER_END_VID_MEM           0xB9000
#define BEFORE_START_KERNEL_MEM     0x3FFFFF
#define START_KERNEL_MEM            0x400000
#define END_KERNEL_MEM              0x7FFFFF
#define AFTER_END_KERNEL_MEM        0x800000

#define FILE_TYPE_0                 0
#define FILE_TYPE_1                 1
#define FILE_TYPE_2                 2


// test launcher
void launch_tests();

#endif /* TESTS_H */
