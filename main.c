#include <stdio.h>

#define SIZE 10

void vInit(int*);  
double vSum(int*);
int vAvg(int*);

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

void vInit(int *vect){

	printf("V = ");
    for(int pos = 0 ; pos < SIZE ; pos++)
    {
        vect[pos] = pos+1;
		printf("%d ", vect[pos]);
    }

	printf("\n");
}

double vSum(int *vect){

    double sum=0;

    for(int pos=0; pos < SIZE ; pos++){
        sum=sum+vect[pos];
    }
    printf("Sum is equal to: %lf \n", sum);
    return sum;
}

// Computes Average
int vAvg(int *array)
{
	int sum = 0;

	for(unsigned int i=0; i < SIZE; i++)
	{
		sum += array[i]; 
	}

	return sum / SIZE;
}

