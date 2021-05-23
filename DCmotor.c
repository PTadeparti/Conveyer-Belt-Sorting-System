/*
 * DCmotor.c
 *
 * Created: 2020-10-27 9:42:56 PM
 *  Author: prash
 */ 
#include <avr/io.h>
#include "DCmotor.h"
#include "timer.h"
#include "lcd.h"

volatile int previousDirection;
	
void turnDCMotor (int direction) {	
	
	// Only want to send the control signal once so check to see if desired motion has switched
	if(direction != previousDirection) {	

		// Send control signals to PORTB bits 0:3.
		switch(direction) {
			case 0:
				PORTB = brake_high;				
				break;
			case 1:
				PORTB = cw;
				break;
			case 2:
				PORTB = ccw;				
				break;
			default:
				PORTB = kill;
		}		
		previousDirection = direction;
	}	
} // turnMotor