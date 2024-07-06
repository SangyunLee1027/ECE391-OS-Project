#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"
#include "file_system.h"

#define PASS 1
#define FAIL 0

#define kernel_page_begin 0x400000
#define kernel_page_end 0x7FFFFF

#define SCREEN_SIZE 	1999

#define vid_mem_begin 0xB8000
#define vid_mem_end 0xB8FFF

#define max_data_size 4096 * 1023 // 4096: 4kbyte, 1023: number of data blocks

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


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* division_error_test
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: division_error
 * Files: idt.c
 */
int division_error_test()	{
	TEST_HEADER;
	int result = PASS;

	int zero = 0;
	int one = 1;
	one = one / zero;
	result = FAIL;

	return result;
}



/* segment_not_present_test
 * 
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: segment_not_present
 * Files: idt.c
 */
int segment_not_present_test()	{
	TEST_HEADER;
	int result = PASS;

	idt[RTC_ADDR].present = 0;
	result = FAIL;

	return result;
}




/* systemcall_error_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: systemcall
 * Files: idt.c
 */
int systemcall_error_test()	{
	TEST_HEADER;
	int result = PASS;

	// 0x80; // need assembly
	__asm__("int	$0x80");
	result = FAIL;

	return result;
}



/* Debug_exception_testing
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: exception_testing
 * Files: idt.c
 */
int Debug_exception_testing()	{
	TEST_HEADER;
	int result = PASS;

	// 0x80; // need assembly
	__asm__("int	$0x1");
	result = FAIL;

	return result;
}

/* Invalid_TSS_exception_testing
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: exception_testing
 * Files: idt.c
 */
int Invalid_TSS_exception_testing()	{
	TEST_HEADER;
	int result = PASS;

	// 0x80; // need assembly
	__asm__("int	$10");
	result = FAIL;

	return result;
}

/* Machine_Check_exception_testing
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: exception_testing
 * Files: idt.c
 */
int Machine_Check_exception_testing()	{
	TEST_HEADER;
	int result = PASS;

	// 0x80; // need assembly
	__asm__("int	$18");
	result = FAIL;

	return result;
}

/* device_not_available_exception_testing
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: exception_testing
 * Files: idt.c
 */
int device_not_available_exception_testing()	{
	TEST_HEADER;
	int result = PASS;

	// 0x80; // need assembly
	__asm__("int	$0x7");
	result = FAIL;

	return result;
}

/* alignment_check_exception_testing
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception: exception_testing
 * Files: idt.c
 */
int alignment_check_exception_testing()	{
	TEST_HEADER;
	int result = PASS;

	// 0x80; // need assembly
	__asm__("int	$0x11");
	result = FAIL;

	return result;
}


/* paging_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: test paging of kernel and video memory (begin and end address)
 * Files: paging.c
 */
int paging_test()	{
	TEST_HEADER;
	char result;
	char* ptr;

	ptr = (char*)kernel_page_begin;	// begin of the kernel page
	result = *ptr;

	ptr = (char*)vid_mem_begin;	// begin of the video memory
	result = *ptr;

	ptr = (char*)kernel_page_end;	// end of the kernel page
	result = *ptr;

	ptr = (char*)vid_mem_end;	// end of the video memory
	result = *ptr;

	return PASS;
}

/* paging_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: test paging of kernel and video memory (middle address)
 * Files: paging.c
 */
int paging_test_2()	{
	TEST_HEADER;
	char result;
	char* ptr;

	ptr = (char*)(kernel_page_begin + 200);	// middle address of the kernel page
	result = *ptr;

	ptr = (char*)(vid_mem_begin + 200);	// middle address of the video memory page
	result = *ptr;

	ptr = (char*)(kernel_page_end - 300);	// middle address of the kernel page
	result = *ptr;

	ptr = (char*)(vid_mem_end- 300);	// middle address of the video memory page
	result = *ptr;

	return PASS;
}


/* kernel_low_boundary_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging out of boundary (low) for kernel
 * Files: paging.c
 */
int kernel_low_boundary_test()	{
	TEST_HEADER;
	char result;
	char* ptr;

	ptr = (char*)(kernel_page_end + 1);	// out of boundary (low)
	result = *ptr;

	return FAIL;
}

/* kernel_up_boundary_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging out of boundary (up) for kernel
 * Files: paging.c
 */
int kernel_up_boundary_test()	{
	TEST_HEADER;
	char result;
	char* ptr;

	ptr = (char*)(kernel_page_begin - 1);	// out of boundary (up)
	result = *ptr;

	return FAIL;
}

/* paging_null_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: test paging for NULL
 * Files: paging.c
 */
int paging_null_test(){
	TEST_HEADER;
	char result;
	char* ptr;

	ptr = NULL;
	result = *(ptr);	// test NULL

	return FAIL;
}

/* vid_mem_up_boundary_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging out of boundary (up) for video memory
 * Files: paging.c
 */
int vid_mem_up_boundary_test()	{
	TEST_HEADER;
	char result;
	char* ptr;

	ptr = (char*)(vid_mem_begin - 1);	// out of bounary (up)
	result = *ptr;

	return FAIL;
}


/* vid_mem_low_boundary_test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging out of boundary (low) for video memory
 * Files: paging.c
 */
int vid_mem_low_boundary_test()	{
	TEST_HEADER;
	char result;
	char* ptr;

	ptr = (char*)(vid_mem_end + 1);	// out of boundary (low)
	result = *ptr;

	return FAIL;
}

// add more tests here

/* Checkpoint 2 tests */


int terminal_readwrite_test() {
	TEST_HEADER;
	terminal_read(0,0,10);
	terminal_write(0,0,10);
	char b[11];
	int x;
	while(1) {
		memset(b,0,11);
		x = terminal_read(0,b,10);
		terminal_write(0,b,x);
	}
	

	return PASS;
}

/* RTC driver test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: RTC open, close, read, write
 * Files: rtc.c
 */
int rtc_read_write_test() {
    TEST_HEADER;
    uint32_t i;
    uint32_t j;
    int32_t result = 0;

    result += rtc_open(0); // "RTC"
	printf("\n");
    for(i = 2; i <= 1024; i *= 2) {
        result += rtc_write(NULL, &i, sizeof(uint32_t));
        printf("Testing: %d Hz [", i);
        for(j = 0; j < i; j++) {
            result += rtc_read(NULL, NULL, NULL);
			printf(".");
        }
        printf("]\n");
    }

	result += rtc_close(0);

    if(result == 0) {
        return PASS;
    } else {
        return FAIL;
    }
}

/* read_data_test1
 * 
 * Inputs: None
 * Outputs: return 0;
 * Side Effects: print the very long data in the file
 * Coverage: can read really long file
 * Files: file_system.c
 */

int read_data_test1(){

	dir_entry_t temp_dentry;
	uint8_t buf[SCREEN_SIZE];
	char fname[] = "verylargetextwithverylongname.txt";
	read_dentry_by_name((uint8_t*)fname, &temp_dentry);
	uint32_t total_byte_read = 0;
	uint32_t byte_read;

	memset(buf, 0, SCREEN_SIZE);

	while(1){
		byte_read = read_data (temp_dentry.inode, total_byte_read, buf, SCREEN_SIZE); // SCREEN_SIZE: size of the file
		printf( "%d", byte_read);
		puts((int8_t*)buf);
		if(byte_read == 0){
			break;
		}
		memset(buf, 0, SCREEN_SIZE);
		total_byte_read += byte_read;
	}


	return 0;
}



/* read_data_test2
 * 
 * Inputs: None
 * Outputs: return 0;
 * Side Effects: print the excutional file
 * Coverage: can read the excutionable file
 * Files: file_system.c
 */

int read_data_test2(){
	int i;

	dir_entry_t temp_dentry;
	uint8_t buf[SCREEN_SIZE];
	char fname[] = "ls";
	read_dentry_by_name((uint8_t*)fname, &temp_dentry);
	uint32_t total_byte_read = 0;
	uint32_t byte_read;

	memset(buf, 0, SCREEN_SIZE);

	while(1){
		byte_read = read_data (temp_dentry.inode, total_byte_read, buf, SCREEN_SIZE); // 4096: size of the file
		for(i=0; i < SCREEN_SIZE; ++i){
			if(buf[i] != 0){
				putc((int8_t)buf[i]);
			}
		}
		if(byte_read == 0){
			break;
		}
		memset(buf, 0, SCREEN_SIZE);
		total_byte_read += byte_read;
	}


	return 0;
}

/* read_data_test3
 * 
 * Inputs: None
 * Outputs: return 0;
 * Side Effects: print the data in short file
 * Coverage: can read the short file
 * Files: file_system.c
 */

int read_data_test3(){

	int i;

	dir_entry_t temp_dentry;
	uint8_t buf[SCREEN_SIZE];
	char fname[] = "fish";
	read_dentry_by_name((uint8_t*)fname, &temp_dentry);
	uint32_t total_byte_read = 0;
	uint32_t byte_read;

	memset(buf, 0, SCREEN_SIZE);

	while(1){
		byte_read = read_data (temp_dentry.inode, total_byte_read, buf, SCREEN_SIZE); // SCREEN_SIZE: size of the file
		printf( "%d", byte_read);
		for(i=0; i < SCREEN_SIZE; ++i){
			if(buf[i] != 0){
				putc((int8_t)buf[i]);
			}
		}
		if(byte_read == 0){
			break;
		}
		memset(buf, 0, SCREEN_SIZE);
		total_byte_read += byte_read;
	}

	return 0;
}

/* read_directories_test
 * 
 * Inputs: None
 * Outputs: return 0;
 * Side Effects: print all directories
 * Coverage: can read all directories
 * Files: file_system.c
 */ 

int read_directories_test(){
	int i;
	uint8_t buf[SCREEN_SIZE];

	memset(buf, 0, SCREEN_SIZE);

	for(i = 0; i < get_num_dir_entry(); ++i){
		dir_read(0, (void*)buf, 0);
		puts((int8_t*)buf);
		printf("\n");
		memset(buf, 0, SCREEN_SIZE);
	}

	return 0;
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//clear();
	// launch your tests here
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("division_error_test", division_error_test());
	// TEST_OUTPUT("segment_not_present_test", segment_not_present_test())
	// TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("paging_test_2", paging_test_2());

	// TEST_OUTPUT("paging_test_2", paging_test_2());
	
	// TEST_OUTPUT("rtc_driver_test", rtc_read_write_test());
	// TEST_OUTPUT("paging_test_2", paging_test_2());

	// TEST_OUTPUT("kernel_up_boundary_test", kernel_up_boundary_test());
	// TEST_OUTPUT("kernel_low_boundary_test", kernel_low_boundary_test());
	// TEST_OUTPUT("vid_mem_low_boundary_test", vid_mem_low_boundary_test());
	// TEST_OUTPUT("vid_mem_up_boundary_test", vid_mem_up_boundary_test());
	// TEST_OUTPUT("paging_null_test", paging_null_test());

	// TEST_OUTPUT("systemcall_test", systemcall_error_test());

	// TEST_OUTPUT("Debug_exception_testing", Debug_exception_testing());
	// TEST_OUTPUT("Invalid_TSS_exception_testing", Invalid_TSS_exception_testing());
	//TEST_OUTPUT("Machine_Check_exception_testing", Machine_Check_exception_testing());
	// TEST_OUTPUT("device_not_available_exception_testing", device_not_available_exception_testing());
	// TEST_OUTPUT("alignment_check_exception_testing", alignment_check_exception_testing());
	


	// TEST_OUTPUT("read_data_test", read_data_test());
	// read_data_test2();

	// TEST_OUTPUT("terminal read/write test", terminal_readwrite_test());
	// read_data_test2();
	// read_data_test3();
	// read_directories_test();
}
