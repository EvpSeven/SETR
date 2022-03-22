#include <stdio.h>
#include <limits.h>
#include "mySAG.h"

int stream[MAXSIZE];
int size = 0;
int n_elements = 0;
int pos = 0;

int MySAGInit(int N)
{
	if(N > MAXSIZE)
	{
		printf("Error Initializing array - N bigger than 100\n");
		return -1;
	}
	for(unsigned int i=0; i<N; i++)
	{
		stream[i] = 0;
	}

	size = N;

	return 0;
}

void MySAGInsert(int val)
{
	stream[pos] = val;
	
	if(n_elements < size)
		n_elements++;
	
	pos = (pos + 1) % size;
}

int MySAGMax()
{
	int max = INT_MIN;

	for(unsigned int i=0; i < n_elements; i++)
	{
		if(stream[i] > max)
			max = stream[i];
	}

	return max;
}

int MySAGMin()
{
	int min = INT_MAX;

	for(unsigned int i=0; i < n_elements; i++)
	{
		if(stream[i] < min)
			min = stream[i];
	}

	return min;
}

double MySAGAvg()
{
	int sum = 0;

	for(unsigned int i=0; i < n_elements; i++)
	{
		sum += stream[i];
	}

	return sum / n_elements;
}

int MySAGFreq(int val)
{
	int count = 0;

	for(unsigned int i=0; i < n_elements; i++)
	{
		if(stream[i] == val)
			count++;
	}

	return count;
}