/*
 * stepper.c
 *
 * Created: 2020-11-26 3:28:28 PM
 *  Author: prash
 */ 
#include <avr/io.h>
#include <math.h>
#include "stepper.h"
#include "timer.h"
#include "LinkedQueue.h"
#include "lcd.h"

extern link *head;
extern link *tail;
link *rlink = 0;

volatile int step = 1;
volatile int currentStep = 0;
volatile int blackFound = 0;
volatile int position; 
int trapAccelLength = 10;
int stepSequence[4] = {step1,step2,step3,step4};

void moveStepper(int direction, int numSteps, int delayms) {
	

	int accelSteps = trapAccelLength; 
	int decelSteps = (numSteps - trapAccelLength);
	int increment = 10;
	int del = 0;

	
	for(int stepCount = 0; stepCount<numSteps; stepCount++) {
		
		// Acceleration portion of trapezoid
		if(stepCount < accelSteps) {
			increment--; // Lower delay, speed up (accelerate)			
		} 
		
		// Deacceleration portion of trapezoid
		if (stepCount > decelSteps) {
			increment++; // Raise delay, slow down (decelerate)
		}
		
		del = delayms + increment; // Update delay to change acceleration
		
		// Clockwise
		if(direction == 0) {
			// Update control signal. Keep track of current step
			if(currentStep == 3) {
				currentStep = 0;
			} else {
				currentStep++;
			}
			PORTA = stepSequence[currentStep];
			mTimer(del);
		}
		
		// CounterClockwise
		if(direction == 1) {
			// Update control signal. Keep track of current step
			if(currentStep == 0) {
				currentStep = 3;
				} else {
				currentStep--;
			}
			PORTA = stepSequence[currentStep];
			mTimer(del);
		}	
	} // for
} // moveStepper

// Uses Hall effect to home to black
void homeToBlack() {	
	
	// Keep rotating until Hall effect sensor interrupt thrown 
	while(!blackFound) {		
		if(currentStep == 3) {
			currentStep = 0;
		} else {
			currentStep++;
		}	
		PORTA = stepSequence[currentStep];	
		mTimer(16);		
	}
} 


void homingComplete() {
	blackFound = 1; // Set flag that homing to black done.
	position = BLACK; // Calibrate position to black after homing	
}

void positionBucket () {
	/* FOR REFERENCE:
	Material Code : aluminum = 1; steel = 2; white = 3; black = 4	
	*/
	
	int material = head->materialCode;
	
	switch(material) {
		
		case 1:
		
			if(position == WHITE) {
				moveStepper(1,STEP_PER360/4,DELAY); // ccw				
			} else if(position == BLACK) {
				moveStepper(0,STEP_PER360/4,DELAY); // cw
			} else if(position == STEEL) {
			moveStepper(0,STEP_PER360/2,DELAY); 
			} else {
			// do nothing
			}
			position = ALUMINUM; // set new position
			addToSortedTotal(); // Done sorting, add to total 
			dequeue(&head,&tail, &rlink); // next item up
			break; 
			
		case 2:
			if(position == WHITE) {
				moveStepper(0,STEP_PER360/4,DELAY); // cw
			} else if(position == BLACK) {
				moveStepper(1,STEP_PER360/4,DELAY); // ccw
			} else if(position == ALUMINUM) {
				moveStepper(0,STEP_PER360/2,DELAY);
			} else {
				// do nothing				
			}
			position = STEEL; // set new position
			addToSortedTotal(); // Done sorting, add to total
			dequeue(&head, &tail, &rlink); // next item up
			break;
		
		case 3:
		
			if(position == ALUMINUM) {
				moveStepper(0,STEP_PER360/4,DELAY); // cw
			} else if(position == BLACK) {
				moveStepper(0,STEP_PER360/2,DELAY); 
			} else if(position == STEEL) {
				moveStepper(1,STEP_PER360/4,DELAY); // ccw
			} else {
				// do nothing
			}
			position = WHITE; // set new position
			addToSortedTotal(); // Done sorting, add to total
			dequeue(&head, &tail, &rlink); // next item up
			break;
			
		case 4:
		
			if(position == WHITE) {
				moveStepper(1,STEP_PER360/2,DELAY); 
			} else if(position == ALUMINUM) {
				moveStepper(1,STEP_PER360/4,DELAY); // ccw
			} else if(position == STEEL) {
				moveStepper(0,STEP_PER360/4,DELAY); // cw
			} else {
				// do nothing
			}
			position = BLACK; // set new position
			addToSortedTotal(); // Done sorting, add to total
			dequeue(&head, &tail, &rlink); // next item up
			break;
			
		default:
			// do nothing	
		break;		
	}
}
