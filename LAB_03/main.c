#include <stdio.h>
#include "mySAG.h"

#define N 200

int main(void)
{
	int val = 5;

	if(MySAGInit(N) == -1)
	{
		return 1;
	}

	for(unsigned int i=0; i<3; i++)
		MySAGInsert(i);

	printf("Min = %d\n", MySAGMin());
	printf("Max = %d\n", MySAGMax());
	printf("Average = %lf\n", MySAGAvg());

	printf("Frequency of %d = %d\n", val, MySAGFreq(val));
	
	return 0;
}