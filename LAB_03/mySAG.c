/** \file mySAG.c
 * 	\brief Module to manipulate stream of integers
 * 
 * \author André Brandão
 * \date 22/03/2022
 */
#include <stdio.h>
#include <limits.h>
#include "mySAG.h"

int stream[MAXSIZE];	/**> Stream to store data */
int size = 0;			/**> Size of the stream gyven by user */
int n_elements = 0;		/**> Number of elements present in the stream */
int pos = 0;			/**> Next position to insert the value in the stream */

/** \brief Function to initialize stream with zeros
 * 
 * \param[in] N Size of stream
 * 
 * \return returns -1 if error initializing stream or 0 otherwise
*/
int MySAGInit(int N)
{
	if(N > MAXSIZE)
	{
		printf("Error Initializing array - N bigger than %d\n", MAXSIZE);
		return -1;
	}

	for(unsigned int i=0; i < N; i++)
	{
		stream[i] = 0;
	}

	size = N;

	return 0;
}

/** \brief Function to insert value in stream
 * 
 * Inserts a value in the first available position.\n
 * If stream is full overwrites oldest value\n
 * The stream is treated as a circular array
 * 
 * \param[in] val value to insert
*/
void MySAGInsert(int val)
{
	stream[pos] = val;
	
	if(n_elements < size)
		n_elements++;
	
	pos = (pos + 1) % size;
}

/** \brief Function to compute MAX value of stream
 * 
 * \return returns maximum value of stream
*/
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

/** \brief Function to compute MIN value of stream
 * 
 * \return returns minimum value of stream
*/
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

/** \brief Function to compute average value of stream
 * 
 * \return returns average value of stream
*/
int MySAGAvg()
{
	int sum = 0;

	for(unsigned int i=0; i < n_elements; i++)
	{
		sum += stream[i];
	}

	return sum / n_elements;
}

/** \brief This function gives the number of times a value is contained in the stream
 * 
 * \return returns number of times the value appear
*/
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