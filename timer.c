/*
 * timer.c
 *
 * Created: 2020-10-27 10:46:33 PM
 *  Author: prash
 */ 
#include <avr/io.h>
#include "timer.h"

void initTimer() {
	CLKPR = 0x80; // Enable Clock prescaling
	CLKPR = 0x01; // Prescale CPU clock (set to 8 MHz)
	TCCR1B |= 1<< CS11; // Set timer prescaler to 8
}

void mTimer(int count) {
	int i = 0;	
	TCCR1B |= 1 << WGM12; // CTC mode (mode 4 in Table 17-2)
	OCR1A = 0x03E8; // Set compare register - 1000 clock cycles
	TCNT1 = 0x0; // Initial timer value = 0
	TIFR1 |= 1 << OCF1A; // clear timer interrupt, begin counting
	while(i<count) {
		if((TIFR1 & 0x02) == 0x02){
			TIFR1 |= 1 << OCF1A; // clear timer interrupt
			i++;
		} // if
	} // while
} // while

