/*
 * stepper.h
 *
 * Created: 2020-11-26 3:26:29 PM
 *  Author: prash
 */ 


#ifndef STEPPER_H_
#define STEPPER_H_


/* 
FOR APPARATUS
Full step 2 phase mode requires energizing both coils at once.
*/


#define step1  0b00110110 // L1, L3
#define step2  0b00101110 // L2, L3
#define step3  0b00101101 // L2, L4
#define step4  0b00110101 // L1, L4
#define DELAY 5
#define BLACK 0
#define ALUMINUM 1
#define WHITE 2
#define STEEL 3
#define STEP_PER360 200



void moveStepper (int direction, int numSteps, int delayms);
void homeToBlack();
void homingComplete();
int calculateSteps (float degrees);
void positionBucket();

#endif /* STEPPER_H_ */