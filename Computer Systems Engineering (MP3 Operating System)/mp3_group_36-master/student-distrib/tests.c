#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "paging.h"
#include "paging_asm.h"

#include "rtc.h"
#include "terminal.h"
#include "file_sys.h"


#define PASS 1
#define FAIL 0
#define MAX_BUF_SIZE 128
#define TOTAL_CHARS 80*25
#define NEG_NUM_1 	-112
#define NEG_NUM_2	-1
#define ONE_NINE	19

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* ----- Checkpoint 1 tests ----- */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			printf("%d\n", i);
			while(1);
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

/* 
	IDT Test 
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int div_by_0_test() {
	int a = 0;
	return 1/a;
}

/*
	Paging Tests:
		1: Generic page fault
		2: Before start of video memory
		3: Start of video memory
		4: End of video memory
		5: After end of video memory
		6: Before start of kernel memory
		7: Start of kernel memory
		8: End of kernel memory
		9: After end of kernel memory

	NOTE:
		Addresses are BYTE addressable. 
			Ptrs point to the beginning of an address
				BUT data casts (uint32, uint16, etc) specify how far from start to look
*/

/*
	Paging Test 1: 		Check whether IDT can return a page fault with a generic NULL ptr.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		rv
*/
int paging_test1() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = NULL;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 2: 		Check if before start of VIDEO MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		Page fault
*/
int paging_test2() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)BEFORE_START_VID_MEM;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 3: 		Check if start of VIDEO MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		No page fault
*/
int paging_test3() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)START_VID_MEM;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 4: 		Check if end of VIDEO MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		No page fault
*/
int paging_test4() { // PROBLEMS
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)END_VID_MEM;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 5: 		Check if after end of VIDEO MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		Page fault
*/
int paging_test5() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)AFTER_END_VID_MEM;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 6: 		Check if before start of KERNEL MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		Page fault
*/
int paging_test6() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)BEFORE_START_KERNEL_MEM;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 7: 		Check if start of KERNEL MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		No page fault
*/
int paging_test7() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)START_KERNEL_MEM;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 8: 		Check if end of KERNEL MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		No page fault
*/
int paging_test8() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)END_KERNEL_MEM;
	deferenced_addr = *addr;
	return PASS;
}

/*
	Paging Test 9: 		Check if after end of KERNEL MEM is set up/valid.
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		IDT, Paging
		RETURN: 		Page fault
*/
int paging_test9() {
	uint8_t *addr; 
	uint8_t deferenced_addr;
	addr = (uint8_t*)AFTER_END_KERNEL_MEM;
	deferenced_addr = *addr;
	return PASS;
}


/* ----- Checkpoint 2 tests ----- */

/*
	test_read_directory:	Read all files, their types, and size
		INPUTS:				n/a
		OUTPUTS:			List of files and their properties
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_read_directory() {
	int fd;
	void* buf = " ";
	int nbytes;
	read_directory(fd, buf, nbytes);
	return PASS;
}

/*
	test_file_driver_read_dentry_by_name_overall:	Read all files, their types, and size by name
 		INPUTS:				n/a
		OUTPUTS:			List of files and their properties
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_file_driver_read_dentry_by_name_overall() {
	dir_entry_t temp_dentry;
	const char* file_name; 
	int ret_val;

	/* regular .txt file that exists */
	file_name = "frame0.txt";
	ret_val = read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	printf("Search for: %s \n", file_name);
	if (ret_val == 0) {
		printf("Pass/Fail: Pass\n");
	}
	else { 
		printf("Pass/Fail: Fail\n");
	}
	printf("File name: %s; ", temp_dentry.file_name);
	printf("File type: %d; ", temp_dentry.file_type);
	printf("File inode count: %d\n\n", temp_dentry.inode);

	/* .txt file that exists but name is too big */
	file_name = "verylargetextwithverylongname.txt";
	ret_val = read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	printf("Search for: %s \n", file_name);
	if (ret_val == 0) {
		printf("Pass/Fail: Pass\n");
	}
	else { 
		printf("Pass/Fail: Fail\n");
	}
	printf("File name: %s; ", temp_dentry.file_name);
	printf("File type: %d; ", temp_dentry.file_type);
	printf("File inode count: %d\n\n", temp_dentry.inode);

	/* .txt file that does not exist */
	file_name = "ZACK_IS_A_LOSER.txt";
	ret_val = read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	printf("Search for: %s \n", file_name);
	if (ret_val == 0) {
		printf("Pass/Fail: Pass\n");
	}
	else { 
		printf("Pass/Fail: Fail\n");
	}
	printf("File name: %s; ", temp_dentry.file_name);
	printf("File type: %d; ", temp_dentry.file_type);
	printf("File inode count: %d\n\n", temp_dentry.inode);

	/* regular file that exists */
	file_name = "fish";
	ret_val = read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	printf("Search for: %s \n", file_name);
	if (ret_val == 0) {
		printf("Pass/Fail: Pass\n");
	}
	else { 
		printf("Pass/Fail: Fail\n");
	}
	printf("File name: %s; ", temp_dentry.file_name);
	printf("File type: %d; ", temp_dentry.file_type);
	printf("File inode count: %d\n\n", temp_dentry.inode);
	
	/* regular file that exists */
	file_name = "grep";
	ret_val = read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	printf("Search for: %s \n", file_name);
	if (ret_val == 0) {
		printf("Pass/Fail: Pass\n");
	}
	else { 
		printf("Pass/Fail: Fail\n");
	}
	printf("File name: %s; ", temp_dentry.file_name);
	printf("File type: %d; ", temp_dentry.file_type);
	printf("File inode count: %d\n\n", temp_dentry.inode);

	return PASS;
}

/*
	test_file_driver_read_dentry_by_index_overall:	Read all files, their types, and size by index
 		INPUTS:				n/a
		OUTPUTS:			List of files and their properties
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_file_driver_read_dentry_by_index_overall() {
	dir_entry_t temp_dentry;
	uint32_t i, ret_val;
	int max_count;
	max_count = 10;	// want 10 files to be printed
	for (i = 0; i < 63; i++) {	// 63 because 63 different detries
		read_dentry_by_index(i, &temp_dentry);
		/* print all files that match the file type */
		if (temp_dentry.file_type == FILE_TYPE_0 || temp_dentry.file_type == FILE_TYPE_1 || temp_dentry.file_type == FILE_TYPE_2) {
			printf("File %d: %s, ", i, temp_dentry.file_name);
			printf("Type: %d, ", temp_dentry.file_type);
			printf("File size : %d\n", (inode_start + temp_dentry.inode)->len_in_B);
			max_count--;
		}
		if (max_count < 1) {
			break;
		}
	}
	printf("\n");

	/* index out of bounds */
	i = 100;	//  arbitrary index number thats out of bounds
	ret_val = read_dentry_by_index(i, &temp_dentry);
	if (ret_val != 0) {
		printf("Index is %u\nFunction failed to find dentry of specific index\n", i);\
	}
	else {
		printf("Function read_dentry_by_index is wrong");
	}

	i = -21;	// arbitrary index number thats out of bounds
	ret_val = read_dentry_by_index(i, &temp_dentry);
	if (ret_val != 0) {
		printf("Index is %u\nFunction failed to find dentry of specific index\n", i);\
	}
	else {
		printf("Function read_dentry_by_index is wrong");
	}
	printf("\n");

	return PASS;
}

/*
	test_read_data1:		Reads all of frame0.txt
 		INPUTS:				n/a
		OUTPUTS:			ASCII fish from MP1
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_read_data1() {	// all fish
	int offset = 0;	
	int length = 250;	// 198 is all of it (length of file)
	uint8_t temp_buf[length];
	dir_entry_t temp_dentry;
	const char* file_name = "frame0.txt";
	read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	read_data(temp_dentry.inode, offset, temp_buf, length);
	int i;
	for (i = 0; i < length; i++) {
		printf("%c",temp_buf[i]);
	}
	return PASS;
}

/*
	test_read_data2:		Reads first couple parts of frame0.txt
 		INPUTS:				n/a
		OUTPUTS:			ASCII fish from MP1
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_read_data2() {	// missing sea floor
	int offset = 0;
	int length = 150;	// 198 is all of it (length of file)
	uint8_t temp_buf[length];
	dir_entry_t temp_dentry;
	const char* file_name = "frame0.txt";
	read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	read_data(temp_dentry.inode, offset, temp_buf, length);
	int i;
	for (i = 0; i < length; i++) {
		printf("%c",temp_buf[i]);
	}
	return PASS;
}

/*
	test_read_data3:		Reads all parts of frame0.txt
 		INPUTS:				n/a
		OUTPUTS:			ASCII fish from MP1
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_read_data3() {	// all fish
	int offset = 0;
	int length = 1000;	// 198 is all of it
	uint8_t temp_buf[length];
	dir_entry_t temp_dentry;
	const char* file_name = "frame0.txt";
	read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	read_data(temp_dentry.inode, offset, temp_buf, length);
	int i;
	for (i = 0; i < length; i++) {
		printf("%c",temp_buf[i]);
	}
	return PASS;
}

/*
	test_read_data4:		Skips first couple parts and reads after those parts of frame0.txt
 		INPUTS:				n/a
		OUTPUTS:			ASCII fish from MP1
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_read_data4() {	// skip first part of fish
	int offset = 50;	// arbitrary number
	int length = 150;	// 198 is all of it
	uint8_t temp_buf[length];
	dir_entry_t temp_dentry;
	const char* file_name = "frame0.txt";
	read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	read_data(temp_dentry.inode, offset, temp_buf, length);
	int i;
	for (i = 0; i < length; i++) {
		printf("%c",temp_buf[i]);
	}
	return PASS;
}

/*
	test_read_data5:		Reads no parts of frame0.txt
 		INPUTS:				n/a
		OUTPUTS:			ASCII fish from MP1
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_read_data5() {	// no fish
	int ret_val;
	int offset = 250;	// arbitary number
	int length = 50;	// 198 is all of it
	uint8_t temp_buf[length];
	dir_entry_t temp_dentry;
	const char* file_name = "frame0.txt";
	read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	ret_val = read_data(temp_dentry.inode, offset, temp_buf, length);
	
	if (ret_val == 0) {
		return PASS;
	}
	int i;
	for (i = 0; i < length; i++) {
		printf("%c",temp_buf[i]);
	}
	return PASS;
}

/*
	test_read_data6:		Reads LENGTH parts of fish
 		INPUTS:				n/a
		OUTPUTS:			File contents of fish
		SIDE EFFECTS:		n/a
		COVERAGE:			File driver & file system
		RETURN:				PASS
*/
int test_read_data6() {
	int offset = 0;	
	/* change this to make it as big/small as desired */
	int length = 200;			// arbitary number
	uint8_t temp_buf[length];
	dir_entry_t temp_dentry;
	const char* file_name = "fish";
	read_dentry_by_name((const uint8_t*)file_name, &temp_dentry);
	read_data(temp_dentry.inode, offset, temp_buf, length);
	int i;
	for (i = 0; i < length; i++) {
		if (temp_buf[i] == '\0')
			continue;
		//printf("%x",temp_buf[i]);	// PRINT HEX VERSIONS OF FISH'S ASCII
		putc(temp_buf[i]);							// type "xxd fish" to print fish's stuff in hex
									// type "xxd fish | less to print less"
									// be in fsdir <- IMPORTANT
	}
	return PASS;
}

/*
	terminal_RW_keyboard_test(): 	should read the first nbytes from terminal and echo when enter is pressed
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		Terminal_read/write
		RETURN: 		Echoed string
*/
int terminal_RW_keyboard_test(){
	int nbytes;
	char buf[TOTAL_CHARS];	
	while(1){
		nbytes = terminal_read(0, buf, MAX_BUF_SIZE);
		terminal_write(0, buf, nbytes);
	}
	return 1;
}

/*
	terminal_write_test(): 	should write the string to terminal and return number of bytes written
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		Terminal_read/write
		RETURN: 		Echoed string
*/
int terminal_write_test(){
	int nbytes;
	// should be 19 characters in total including the newline char
	nbytes = terminal_write(0, (void*)"ECE 391 experience\n", MAX_BUF_SIZE);
	printf("Num of bytes printed: %d\n", nbytes);
	if(nbytes == ONE_NINE) 
		return 1;
	else
		return 0;
}

/*
	terminal_write_test(): 	should write the string to terminal and return number of bytes written
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		Terminal_read/write
		RETURN: 		Echoed string
*/
int terminal_open_close_test(){
	int open;
	int close;
	int32_t fd;
	const uint8_t* name;
	open = terminal_open(name);
	if(open == 0)
		printf("Open Sucess\n");
	close = terminal_close(fd);
	if(close == 0){
		printf("Close sucess\n");
	}
	if(close == 0 && open == 0)
		return 1;
	else
		return 0;
}
/*
	terminal_write_test(): 	should write the string to terminal and return number of bytes written
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		Terminal_read/write
		RETURN: 		Echoed string
*/
int terminal_RW_Error(){
	int read_retval, write_retval;
	char buf[TOTAL_CHARS];
	read_retval = terminal_read(0, buf, NEG_NUM_1);
	write_retval = terminal_write(0, buf, NEG_NUM_2);
	if(read_retval == -1 && write_retval == -1){
		printf("Read and write have received negative byte inputs, please fix\n");
		return 1;
	}
	else	
		return 0;
}

/*
	terminal_NULL_BUF(): 	should write the string to terminal and return NULL ptr message
		INPUTS:			n/a
		OUTPUTS:		n/a
		SIDE EFFECTS:	n/a
		COVERAGE:		Terminal_read/write
		RETURN: 		Echoed string
*/
int terminal_NULL_BUF(){
	int read_retval, write_retval;
	char * buf = NULL;
	read_retval = terminal_read(0, buf, MAX_BUF_SIZE);
	write_retval = terminal_write(0, buf, MAX_BUF_SIZE);
	if(read_retval == -1 && write_retval == -1){
		printf("NULL Buf entered\n");
		return 1;
	}
	else	
		return 0;
}

/* RTC Read/Write/Open/Close Test - Example
 * 
 * Writes and Read the RTC and prints with interupts
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: RTC_Read, RTC_Write, RTC_Open, & RTC_Close
 * Files: rtc.h/rtc.c
 */
int test_rtc_write(){
	int i,j;
	uint32_t fd;
	const uint8_t* filename;
	int32_t nbytes;
	rtc_open(filename);// reset rtc to 2hz
	clear(); // clears terminal
	for( i = 2; i <= 1024; i = i*2){ // initial loop to increase rtc by multiples of 2
		for(j = 0; j <= i; j++ ){ // loop to print from 0 to freq
				rtc_read(fd, (void*) i, nbytes);
				printf("1");
		}
		rtc_write(fd, (void*) i, nbytes);
		clear();
	}
	rtc_close(fd); // close rtc
	return 1;
	//return ((retval == -1) ? retval:!retval);
}













/* Checkpoint 3 tests */

// // Test opening a file
// int test_file_sys_1() {
// 	int8_t temp_buf[200];
// 	char* file_name = "frame0.txt";
// 	printf("Read before opening: %d\n", file_sys_read(2, temp_buf, 200));
// 	printf("Open %s: %d\n", file_name, file_sys_open(file_name));
// 	printf("Read after opening: %d\n",file_sys_read(2, temp_buf, 200));
// 	printf("Buf contents: \n%s", temp_buf);
// 	int i;
// 	printf("File Descriptor Table Inodes: ");
// 	for (i = 0; i < 8; i++) {
// 		printf("%d, ",file_desc_table[i].inode);
// 	}
// 	return 1;
// }

// // Test opening and closing a file
// int test_file_sys_2() {
// 	int8_t temp_buf[200];
// 	char* file_name = "frame0.txt";
// 	printf("Read before opening: %d\n", file_sys_read(2, temp_buf, 200));
// 	printf("Open %s: %d\n", file_name, file_sys_open(file_name));
// 	printf("Read after opening: %d\n",file_sys_read(2, temp_buf, 200));
// 	//printf("Buf contents: \n%s", temp_buf);
// 	int i;
// 	printf("File Descriptor Table Inodes: ");
// 	for (i = 0; i < 8; i++) {
// 		printf("%d, ",file_desc_table[i].inode);
// 	}
// 	printf("\nClose %s: %d\n", file_name, file_sys_close(file_name));
// 	printf("Read after closing: %d\n", file_sys_read(2, temp_buf, 200));
// 	printf("File Descriptor Table Inodes: ");
// 	for (i = 0; i < 8; i++) {
// 		printf("%d, ",file_desc_table[i].inode);
// 	}
// 	return 1;
// }

// // Test opening a file multiple times
// int test_file_sys_3() {
// 	int8_t temp_buf[200];
// 	char* file_name = "frame0.txt";
// 	printf("Read before opening: %d\n", file_sys_read(2, temp_buf, 20));
// 	printf("Open %s: %d\n", file_name, file_sys_open(file_name));
// 	printf("Read 20 bytes: %d\n",file_sys_read(2, temp_buf, 20));
// 	printf("Buf contents: %s\n", temp_buf);
// 	printf("Read 20 bytes: %d\n",file_sys_read(2, temp_buf, 20));
// 	printf("Buf contents: %s\n", temp_buf);
// 	printf("Read 20 bytes: %d\n",file_sys_read(2, temp_buf, 20));
// 	printf("Buf contents: %s\n", temp_buf);
// 	printf("Read 1000 bytes: %d\n",file_sys_read(2, temp_buf, 1000));
// 	printf("Buf contents: %s\n", temp_buf);
// 	printf("Read 1000 bytes: %d\n",file_sys_read(2, temp_buf, 1000));
// 	return 1;
// }

// // Test opening more than 6 files
// int test_file_sys_4() {
// 	return 1;
// }

// // Test opening a file that doesn't exist
// int test_file_sys_5() {
// 	return 1;
// }

// // Test closing a file that is not in the file descriptor table
// int test_file_sys_6() {
// 	return 1;
// }



/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests() {
	clear();
	/* ----- CHECKPOINT 1 TEST LAUNCHES ----- */
	/* IDT Tests */
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("div_by_0_test", div_by_0_test());
		
		/* PAGING Tests */
			//TEST_OUTPUT("paging_test1", paging_test1());	// fail
			//TEST_OUTPUT("paging_test2", paging_test2());	// fail
			// TEST_OUTPUT("paging_test3", paging_test3());	// pass
			// TEST_OUTPUT("paging_test4", paging_test4());	// pass
			//TEST_OUTPUT("paging_test5", paging_test5());	// fail
			//TEST_OUTPUT("paging_test6", paging_test6());	// fail
			// TEST_OUTPUT("paging_test7", paging_test7());	// pass
			// TEST_OUTPUT("paging_test8", paging_test8());	// pass
			//TEST_OUTPUT("paging_test9", paging_test9());	// fail
	
	/* ----- CHECKPOINT 2 TEST LAUNCHES ----- */
		/* FILE SYS Tests */
			//TEST_OUTPUT("test_read_directory", test_read_directory());
			//TEST_OUTPUT("test_file_driver_read_dentry_by_name_overall", test_file_driver_read_dentry_by_name_overall());
			//TEST_OUTPUT("test_file_driver_read_dentry_by_index_overall", test_file_driver_read_dentry_by_index_overall());
			//TEST_OUTPUT("test_read_data1", test_read_data1());
			//TEST_OUTPUT("test_read_data2", test_read_data2());
			// TEST_OUTPUT("test_read_data3", test_read_data3());
			//TEST_OUTPUT("test_read_data4", test_read_data4());
			//TEST_OUTPUT("test_read_data5", test_read_data5());
			//TEST_OUTPUT("test_read_data6", test_read_data6());

		/* RTC Tests */
			//TEST_OUTPUT("test_rtc_write", test_rtc_write());	// works

		/* KEYBOARD/TERMINAL Tests */
			//TEST_OUTPUT("terminal_RW_keyboard_test", terminal_RW_keyboard_test());
			// TEST_OUTPUT("terminal_write_test", terminal_write_test());
			// TEST_OUTPUT("terminal_open_close_test", terminal_open_close_test());
			// TEST_OUTPUT("terminal_RW_Error", terminal_RW_Error());
			// TEST_OUTPUT("terminal_NULL_BUF", terminal_NULL_BUF());

	/* ----- CHECKPOINT 2 TEST LAUNCHES ----- */
		/* FILE SYS Tests */
			//TEST_OUTPUT("test_file_sys_1", test_file_sys_1());
			//TEST_OUTPUT("test_file_sys_2", test_file_sys_2());
			//TEST_OUTPUT("test_file_sys_3", test_file_sys_3());




}
