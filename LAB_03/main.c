/** \file main.c
 * 	\brief This file tests the module mySAG
 * 
 * \author André Brandão
 * \author Emanuel Pereira
 * \date 22/03/2022
 */
#include <stdio.h>
#include "mySAG.h"


int main(void)
{
	int N=10; //value of array size
	int val = 2; 
	printf("Using array SIZE, N= %d: \n", N);
	// Inits the module
	if(MySAGInit(N) == -1)
		return 1;
	
	printf("vector=[ ");
	// Adds an element to the stream
	for(unsigned int i=0; i<N; i++){
		MySAGInsert(i);
		printf("%d ", i);
	}
	printf("]\n");

	printf("Min = %d\n", MySAGMin()); //minimum value of the stream window
	printf("Max = %d\n", MySAGMax()); //maximum value of the stream window
	printf("Average = %d\n", MySAGAvg()); //average of the values of the stream window
	printf("Frequency of %d = %d\n", val, MySAGFreq(val)); //number of times that val appears
	
	printf("################################\n");
	N=200;
	printf("Using bigger array SIZE then allowed, N= %d \n", N);
	if(MySAGInit(N) == -1)
		return 1;
	

	return 0;
}