#include "mySAG.h"
#include <limits.h>

int MySAGMax(int *stream, int N)
{
	int max = INT_MIN;

	for(unsigned int i=0; i < N; i++)
	{
		if(stream[i] > max)
			max = stream[i];
	}

	return max;
}

int MySAGMin(int *stream, int N)
{
	int min = INT_MAX;

	for(unsigned int i=0; i < N; i++)
	{
		if(stream[i] < min)
			min = stream[i];
	}

	return min;
}

double MySAGAvg(int *stream, int N)
{

	int sum = 0;

	for(unsigned int i=0; i < N; i++)
	{
		sum += stream[i];
	}

	return sum / N;
	
}