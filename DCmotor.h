/*
 * DCmotor.h
 *
 * Created: 2020-10-27 9:43:16 PM
 *  Author: prash
 */ 


#ifndef DCMOTOR_H_
#define DCMOTOR_H_

#define cw 0b1011
#define ccw 0b0111
#define brake_high 0b1111
#define kill 0b0000


void turnDCMotor (int direction);

#endif /* DCMOTOR_H_ */