/** \file main.c
 * \brief Program to manipulate array
 * 
 * Initializes an array of \ref SIZE elements with natural numbers starting from 1 to SIZE.\n 
 * Computes and prints sum and average of the array.
 * 
 *	\author André Brandão
 *	\author Emanuel Pereira
 *	\date 15/03/2022
*/

#include <stdio.h>

/** \brief Size of the vector
 * \def SIZE
*/
#define SIZE 10

void vInit(int*);  
double vSum(int*);
double vAvg(int*);

int main(void)
{
    int vect[SIZE]; //init array
	double average=0;

    vInit(vect);
    vSum(vect);
	average = vAvg(vect);

	printf("Average = %lf \n", average);
    return 0;

}

/** \brief This function initialize array with \ref SIZE natural numbers starting from 1 to \ref SIZE
*
* \author Emanuel Pereira
* \param[in] vect Blank array with \ref SIZE numbers
*/
void vInit(int *vect){

	printf("V = ");
    for(int pos = 0 ; pos < SIZE ; pos++)
    {
        vect[pos] = pos+1;
		printf("%d ", vect[pos]);
    }

	printf("\n");
}


/** \brief Sum of array with \ref SIZE numbers
*
* \author Emanuel Pereira
*
* \param[in] vect Array with \ref SIZE numbers
* \return sum of the array
*/
double vSum(int *vect){

    double sum=0;

    for(int pos=0; pos < SIZE ; pos++){
        sum=sum+vect[pos];
    }
    printf("Sum is equal to: %lf \n", sum);
    return sum;
}

/** \brief Computes the average of the array
 * 
 * \author André Brandão
 * 
 * \param[in] array array with \ref SIZE elements
 * \return average of the array
*/
double vAvg(int *array)
{
	int sum = 0;

	for(unsigned int i=0; i < SIZE; i++)
	{
		sum += array[i]; 
	}

	return sum / SIZE;
}