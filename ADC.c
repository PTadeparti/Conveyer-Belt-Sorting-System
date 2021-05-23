#include <avr/interrupt.h>
#include <avr/io.h>


volatile unsigned int ADC_result;

#include "ADC.h"
#include "timer.h"

void initADC() {	
	ADCSRA |= (1 << ADEN); // Enable ADC
	ADCSRA |= (1 << ADIE); // Enable ADC interrupts.
	ADMUX |= (1 << REFS0) | (1 << MUX0); // AVCC as voltage reference. ADC1 channel
} // initADC

// Just for ease of use. Sets enable bit
void startConversion() {
	ADCSRA |= (1 << ADSC); 
}

// Assign 10 bit value to integer variable
ISR(ADC_vect) {
	ADC_result = ADC;
}

// Getter for ADC Result
int getADCResult () {
	return ADC_result;
}




