/*
Inspection System - Sorting Apparatus
Mech 458 Final Project
Authors: Prashant Tadeparti & Evan Douglas Gray


*/

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>


#include "ADC.h"
#include "PWM.h"
#include "DCmotor.h"
#include "timer.h"
#include "lcd.h"
#include "ReflectiveSensor.h"
#include "LinkedQueue.h"
#include "stepper.h"

// function prototypes
int numItemsOnBelt();
void displayTotalSortedItems();
void initInterrupt();
void rampDownTimer();

// Global Variables
volatile char STATE;
volatile int paused = 0;
volatile int numSortedAluminum;
volatile int numSortedSteel;
volatile int numSortedBlack;
volatile int numSortedWhite;
volatile int rampdownCount = 0;
link *head;
link *tail;


int main(int argc, char *argv[]){
	
	
	//Initialize Peripherals
	InitLCD(LS_BLINK|LS_ULINE);	
	initTimer();
	initADC();
	setupList();
	initPWM();
	initInterrupt();
	
	DDRA = 0xFF;
	DDRD = 0b11110000;	// Going to set up INT1, INT2 & INT3 on PORTD
	DDRC = 0xFF;		// just use as a display
	DDRE = 0b11001111; // INT4 as Pause input, INT5 as Rampdown input
	//PORTE = 0b00110000; // Pullup resistors needed for buttons on home setup	

	STATE = 0; // set state to polling initially	
	int LCDwritten = 0; // to avoid overwriting LCD which makes it flicker	
	
	homeToBlack(); // Set up sorting tray in correct position (black facing conveyor)
	turnDCMotor(0); // Conveyor stopped initially

	//Clear the LCD
	LCDClear();

	// Basic startup message
	LCDWriteString("System Startup");
	mTimer(1000);	
	LCDClear();		

	// start actual operation
	goto POLLING_STAGE;

	// 	POLLING STATE
	// 	Wait for interrupts from optical sensors, pause, or ramp down
	POLLING_STAGE:
		
		turnDCMotor(2); // Keep conveyor belt moving forwards. Polling works by waiting until object in sensor view.	
		
		switch(STATE){
			case (0) :
			goto POLLING_STAGE;
			break;	//not needed but syntax is correct		
			case (2) :
			goto REFLECTIVE_STAGE;		
			break;
			case (3) :
			goto PAUSED_STAGE;
			break;
			case (4) :
			goto BUCKET_STAGE;
			break;
			case (5) :
			goto END;
			default :
			goto POLLING_STAGE;
		}//switch STATE

	
	/* REFLECTIVE STATE
	* Triggered on interrupt from OR1
	* Checks reflectiveness and sorts items. Puts all items into a linked list.
	*/
	REFLECTIVE_STAGE:	
		reflectiveStage();		
		LCDwritten = 0; // reset LCD variable so that it can be written again
		STATE = 0; // Reset state variable	
		goto POLLING_STAGE;
	
	/* BUCKET STATE
	* Triggered on interrupt from EX
	* Uses stepper motor to turn bucket in order for sorted items to go into the correct place
	*/	
	BUCKET_STAGE:	
		turnDCMotor(0);	// Stop conveyor	
		positionBucket();	
		LCDwritten = 0; // reset LCD variable so that it can be written again
		STATE = 0; // Reset state variable	
		goto POLLING_STAGE;
	
	/* PAUSED STATE
	* Triggered on interrupt from INT4 
	* Stops motor and pauses functionality. Displays sorted items and # of items currently on belt.
	*/
	PAUSED_STAGE:
		turnDCMotor(0); // Stop conveyor
		LCDClear();
		LCDwritten = 0;
	
		while(paused) {		
			if(!LCDwritten) {		
				displayTotalSortedItems();
				LCDwritten = 1;		
				}
			}	
		STATE = 0;
		LCDwritten = 0;	
		goto POLLING_STAGE;
	
	/* END
	After Ramp down timer finished counting down
	*/
	END:
		turnDCMotor(0); // brake motor for 20 ms
		mTimer(20);
		turnDCMotor(99); // kill DC motor	
		LCDClear();
		displayTotalSortedItems();
	
	return(0);

}


/* ISR triggered by HE - Hall effect sensor */
ISR(INT1_vect) {
	homingComplete(); // Set a flag to stop the stepper
	EIMSK &= ~(1 << INT1); // Turn off interrupts for Hall effect
}

/* ISR triggered by OR1 - Object in Reflective Sensor View  */
ISR(INT2_vect) {	
	startConversion(); // start the ADC conversion for reflective sensor
	STATE = 2;	// Go to Reflective Stage
}


/* ISR triggered by EX - Object in Exit Sensor View */
ISR(INT3_vect){		
	STATE = 4; // Go to Bucket Stage	
}

/* ISR triggered by Pause push button  */
ISR(INT4_vect) {
	turnDCMotor(0); // Stop conveyor	
	while((PINE & 0b00010000) == 0); // wait for release of button
	mTimer(10); // debounce
	
	// Check current status and flip. Unpause if paused and pause if unpaused
	if(paused == 0) {
		paused = 1;
		STATE = 3;	
	} else {
		paused = 0;
		STATE = 0;
	}	
}

/* ISR triggered by Ramp down push button  */
ISR(INT5_vect){
	
	mTimer(10); // debounce	
		
	// Turn off pause and ramp down interrupts	
	EIMSK &= ~(1 << INT4);
	EIMSK &= ~(1 << INT5);
	
	rampDownTimer(); // Start background timer to count down from 8 seconds	
}

// If an unexpected interrupt occurs (interrupt is enabled and no handler is installed,
// which usually indicates a bug), then the default action is to reset the device by jumping
// to the reset vector. You can override this by supplying a function named BADISR_vect which
// should be defined with ISR() as such. (The name BADISR_vect is actually an alias for __vector_default.
// The latter must be used inside assembly code in case <avr/interrupt.h> is not included.
ISR(BADISR_vect)
{
	while(1) {
		PORTC = 0xFF;
		mTimer(100);
		PORTC = 0x00;
		mTimer(100);
	}
}

// Updates the total number of items sorted for each material
void addToSortedTotal() {
	switch(head->materialCode) {
		case 1:
			numSortedAluminum++;
			break;
		case 2:
			numSortedSteel++;
			break;
		case 3:
			numSortedWhite++;
			break;
		case 4:
			numSortedBlack++;
			break;
		default:
			break;
	}	
}


// Gets number of items currently on belt 
int numItemsOnBelt() {
	return size(&head,&tail);
}

// Displays all sorting numbers since startup 
void displayTotalSortedItems() {
	LCDClear();
	LCDWriteString("AL ST BL WH TBS");
	LCDWriteIntXY(0,1,numSortedAluminum,2);
	LCDWriteIntXY(3,1,numSortedSteel,2);
	LCDWriteIntXY(6,1,numSortedBlack,2);
	LCDWriteIntXY(9,1,numSortedWhite,2);
	LCDWriteIntXY(12,1,size(&head, &tail),2);					
}

void initInterrupt() {
	cli();		// Disables all interrupts
		
	// Set up the Interrupt 1,2,3,4,5	
	EICRA |= 1 << ISC11; // HE - PD1 - INT1 -  Active LO
	EICRA |= (1 << ISC21) | (1 << ISC20); // OR - PD2 - INT2 - Active HI
	EICRA |= 1 << ISC31; // EX - PD3 - INT3 - Active Lo
	EICRB |= 1 <<ISC41; // PAUSE - INT4 - PE4 - Active Lo
	EICRB |= 1 <<ISC51; // RAMPDOWN - INT5 - PE5 - Active Lo
	EIMSK |=  (1 << INT1) | (1 << INT2) | (1 << INT3) | (1 << INT4) | (1 << INT5);		
	sei();	// Note this sets the Global Enable for all interrupts	
}

void rampDownTimer(){
	cli();//stop interrupts	
	//set timer4 interrupt at 1Hz
	TCCR4A = 0x00;	
	TCNT4  = 0;//initialize counter value to 0	
	TCCR4B |= (1 << WGM12); // CTC mode	
	TCCR4B |= (1 << CS12) | (1 << CS10); // 1024 prescaler - makes it run at 7812 Hz (1 clock cycle = 0.000128 seconds)	
	OCR4A = 0xFF00;	// (65280 clock cycles)*(0.000128 seconds per cycle) = ~8 seconds
	TIMSK4 |= (1 << OCIE4A); // enable timer compare interrupt
	sei();//allow interrupts
}

ISR(TIMER4_COMPA_vect){	
	STATE = 5;
}





