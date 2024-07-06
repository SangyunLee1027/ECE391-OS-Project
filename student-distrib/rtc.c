/* rtc.c - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */
#include "rtc.h"

volatile uint32_t rtc_interrupt = 0;

// rtc_init
//Input: None
//Output: None
//Effects: initialize the rtc interrupt
// Note: Disable ALL interrupts beforehand, or rtc can enter an
// undefined state that a cold reboot cannot fix
void rtc_init(void) {

    long flags;

    seconds = 0;
    minutes = 0;
    hours = 0;

    cli_and_save(flags);

    // use Status register A to set the rate frequency


    //enable period interrupt at default 1024Hz, irq handler MUST be installed beforehand
    outb(INIT2_RTC, RTC_PORT);
    char prev = inb(CMOS_PORT);
    outb(INIT2_RTC, RTC_PORT);
    outb(INIT2_CMOS | prev, CMOS_PORT);

    enable_irq(8);


    restore_flags(flags);

}

/*
    Changes the RTC interrupt rate
    Input: rate - value 1-15 which divides the frequency
    Output: none
    Effects: Changes rate which RTC sends interrupts
    Notes: apparently the rates of 1 or 2 has a "roll over" problem
    where the interrupts are 0.81Ms and 3.91mS instead of the expected 61uS and 30.5uS.
    So fastest rate that should be used is 3, which is 122uS
*/
void rtc_change_frequency(char rate) {
    long flags;
    rate &= 0x0F;
    cli_and_save(flags);
    
    /*
        Frequency = 0x8000 >> (rate-1)
    */
    outb(REG_A, RTC_PORT);
    char prev = inb(CMOS_PORT);

    outb(REG_A, RTC_PORT);
    outb( (prev & 0xf0) | rate, CMOS_PORT);

    restore_flags(flags);
}

/*
    Changes rtc_irq_handler
    Input: None
    Output: none
    Effects: handle the rtc interrupt and updates the time. 
*/
void rtc_irq_handler(void) {

    rtc_interrupt = 1; /* interrupt is occurring */

    /*
        CMOS register C contains bitmask of which interrupt happened.
        If C is not read after an IRQ 8, the interrupt will never happen again.
        Not needed at the moment, so data is tossed.
    */
    outb(0x0C, RTC_PORT);
    inb(CMOS_PORT);

    // Send End Of Interrupt back to PIC (probably)
    send_eoi(RTC_IRQ);

}

/*
    Changes rtc_get_time
    Input: None
    Output: the current time in seconds
    Effects: None
*/
long rtc_get_time(void) {
    long s = seconds;
    long m = minutes;
    long h = hours;
    return s + m*60 + h*360; /* time conversion work */
}

/*
    Initializes RTC frequency to 2Hz
    Input: const uint8_t* filename
    Output: return 0
*/
int32_t rtc_open(const uint8_t* filename) {
    long flags;
    cli_and_save(flags);
    char prev = inb(CMOS_PORT);
    outb(REG_A, RTC_PORT);
    outb( HERTZ_2 | (prev & 0xf0), CMOS_PORT); 
    restore_flags(flags);
    return 0;
}

/*
    Closes RTC; does nothing
    Input: int32_t fd
    Output: return 0
*/
int32_t rtc_close(int32_t fd) {
    return 0;
}

/*
    Blocks until next interrupt
    Input: fd - file descriptor
           buf - input data pointer
           nbytes - number of bytes
    Output: return 0
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    while(rtc_interrupt == 0) {asm volatile ("hlt");};
    rtc_interrupt = 0;
    return 0;
}

/*
    Writes data to the RTC device; system only accept a 4 byte integer specifying the interrupt rate in Hz and set rate accordingly.
    Frequencies must be power of 2
    Input: fd - file descriptor
           buf - input data pointer
           nbytes - number of bytes
    Output: return 0 or -1
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)  {
    if(buf == NULL || nbytes != 4) { /* if buf is empty or number of bytes not 4, fail*/
        return 0;
    }

    int32_t freq = *((int32_t*)buf);
    if ((freq & (freq - 1)) != 0) { /* if frequency not power of 2, fail */
        return 0;
    }

    int32_t rate = freq_rate(freq);
    if (rate == -1) { /* frequency not valid, fail */
        return 0;
    }
    
    long flags;
    cli_and_save(flags);

    char prev = inb(CMOS_PORT);
    rate = rate | (prev & 0xf0);

    outb(REG_A, RTC_PORT);
    outb(rate, CMOS_PORT); /* set desired rate */

    restore_flags(flags);

    return nbytes;
}

/*
    helper function to change frequency to rate
    Input: frequency (int)
    Output: translated rate on success or -1 on fail
*/
int32_t freq_rate(int32_t freq){
    switch (freq){
        case 2:     return HERTZ_2;
        case 4:     return 0x0E;
        case 8:     return 0x0D;
        case 16:    return 0x0C;
        case 32:    return 0x0B;
        case 64:    return 0x0A;
        case 128:   return 0x09;
        case 256:   return 0x08;
        case 512:   return 0x07;
        case 1024:  return 0x06;
        default:    return -1;
    }
}
