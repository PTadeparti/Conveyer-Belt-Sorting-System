/*
 * ReflectiveSensor.h
 *
 * Created: 2020-11-16 11:25:21 AM
 *  Author: prash
 */ 


#ifndef REFLECTIVESENSOR_H_
#define REFLECTIVESENSOR_H_


#define ALUMINUM_MIN 0
#define ALUMINUM_MAX 350
#define STEEL_MIN 351
#define STEEL_MAX 770
#define WHITE_MIN 771
#define WHITE_MAX 955
#define BLACK_MIN 956
#define BLACK_MAX 1023

void reflectiveStage();
void addToSortedList (int material);
int sortItem(int reflectiveResult);
void setupList();
void show();
void printItemType(int materialKey);

	


#endif /* REFLECTIVESENSOR_H_ */