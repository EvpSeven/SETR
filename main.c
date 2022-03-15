#include <stdio.h>

#define SIZE 10

int vAvg(int*);

int main(void)
{
	double average=0;

	average = vAvg(array);

	printf("Average = %lf \n", average);

	return 0;
}

// Computes Average
int vAvg(int *array)
{
	int sum=0;

	for(unsigned int i=0; i < SIZE; i++)
	{
		sum += array[i]; 
	}

	return sum / SIZE;
}