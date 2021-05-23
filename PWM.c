/*
 * PWM.c
 *
 * Created: 2020-10-27 9:48:52 PM
 *  Author: prash
 */ 
#include <avr/interrupt.h>
#include <avr/io.h>
#include "PWM.h"
#include "ADC.h"

void initPWM() {	
	DDRB |= 0xFF; // set Port B to output (OCR0A is in port B).
	TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00); // Set to fast PWM and compare match to clear at TOP.
	OCR0A = 0x60; // ~40% duty cycle
	TCCR0B |= (1 << CS01) | (1 << CS00); // set clock prescale to 8	
}

