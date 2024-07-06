/* rtc.h - Defines used in interactions with the RTC 
 * controller
 * The rtc is linked to IRQ 8 in the PIC
 *
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

 #include "types.h"
 #include "i8259.h"
 #include "lib.h"

#ifndef ASM

 #define RTC_PORT   0x70
 #define CMOS_PORT  0x71
 #define RTC_IRQ    0x08

 #define INIT1_RTC  0x8A
 #define INIT1_CMOS 0x20
 #define INIT2_RTC  0x8B
 #define INIT2_CMOS 0x40

 #define REG_A          0x8A
 #define REG_SEC        0x00
 #define REG_MIN        0x02
 #define REG_HOUR       0x04
 #define REG_WEEKDAY    0x06 //unreliable
 #define REG_DAYMONTH   0x07
 #define REG_MONTH      0x08
 #define REG_YEAR       0x09

 #define HERTZ_2        0x0F

long seconds;
long minutes;
long hours;

// Initialize the RTC
void rtc_init(void);

// Change interrupt rate
void rtc_change_frequency(char rate);

// Handle irq
void rtc_irq_handler(void);

// Get total recorded seconds
long rtc_get_time(void);

// open rts, initialize rate to 2
int32_t rtc_open(const uint8_t* filename);

// close rtc; do nothing
int32_t rtc_close(int32_t fd);

// wait until interrupt
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

// change freuency based on buffer
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

// helper function to change frequency to rate for rtc_write
int32_t freq_rate(int32_t freq);

#endif
#endif
