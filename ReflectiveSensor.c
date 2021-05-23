/*
 * ReflectiveSensor.c
 *
 * Created: 2020-11-16 11:25:37 AM
 *  Author: prash
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

#include "ADC.h"
#include "lcd.h"
#include "timer.h"
#include "LinkedQueue.h"
#include "ReflectiveSensor.h"

volatile int minimumReflectiveValue;
volatile int itemMaterialKey;
int minimumReached;


extern link *head;
extern link *tail;
link *newlink = NULL;
link *returnlink = NULL;


void reflectiveStage() {
	minimumReflectiveValue = 5000;	// Set some arbitrary large value.

	// While object still in OR View
	while((PIND & 0x04) != 0) {
		startConversion(); // Continue next ADC conversion		
		
		// check for minimum. If new ADC value is less than minimum, replace it
		if(getADCResult() < minimumReflectiveValue ) {
			minimumReflectiveValue = getADCResult();			
		}			
	}
	
	// This only happens after each object has left the OR sensor view.
	ADCSRA &= ~(1 << ADSC); // Stop conversion
	itemMaterialKey = sortItem(minimumReflectiveValue); // Assign a "material key" to the item. This classifies the material.
	addToSortedList(itemMaterialKey);
}

void addToSortedList (int materialkey) {	
	initLink(&newlink);
	newlink->materialCode = materialkey;
	enqueue(&head, &tail, &newlink);	
}

void setupList() {
	setup(&head, &tail);
}

int sortItem(int reflectiveResult) {
	
	if((reflectiveResult < ALUMINUM_MAX) && (reflectiveResult >= ALUMINUM_MIN)) {
		return 1;		
	} else if((reflectiveResult <= STEEL_MAX) && (reflectiveResult >= STEEL_MIN)) {
		return 2;		
	} else 	if((reflectiveResult <= WHITE_MAX) && (reflectiveResult >= WHITE_MIN)) {
		return 3;
	} else 	if((reflectiveResult <= BLACK_MAX) && (reflectiveResult >= BLACK_MIN)) {
		return 4;
	} else {
		return 99;
	}
}




