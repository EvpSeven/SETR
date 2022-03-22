/** \file main.c
 * 	\brief This file tests the module mySAG.h
 * 
 * \author André Brandão
 * \date 22/03/2022
 */
#include <stdio.h>
#include "mySAG.h"

/** Size of the Stream
 * \def N
 */ 
#define N 5

int main(void)
{
	int val = 5;

	if(MySAGInit(N) == -1)
		return 1;

	for(unsigned int i=0; i<3; i++)
		MySAGInsert(i);

	printf("Min = %d\n", MySAGMin());
	printf("Max = %d\n", MySAGMax());
	printf("Average = %lf\n", MySAGAvg());

	printf("Frequency of %d = %d\n", val, MySAGFreq(val));
	
	return 0;
}