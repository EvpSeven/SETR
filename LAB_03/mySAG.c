#include "mySAG.h"
#include <limits.h>

int* MySAGInit(int N)
{
	int stream[N];
	
	for(unsigned int i=0; i<N; i++)
	{
		stream[i] = 0;
	}

	return stream;
}

int MySAGInsert(int *stream, int N, int val)
{
	static int pos = 0;

	stream[pos] = val;
	
	pos = (pos + 1) % N;
}

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

int MySAGFreq(int *stream, int N, int val)
{
	int count = 0;

	for(unsigned int i=0; i < N; i++)
	{
		if(stream[i] == val)
			count++;
	}

	return count;
}