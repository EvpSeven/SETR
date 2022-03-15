#include <stdio.h>
#define size 10

int vInit(int*);  
double vSum(int*);

int main(void)
{
    int vect[size]; //init array

    vInit(vect);
    vSum(vect);
    return 0;

}

int vInit(int *vect){

    for(int pos = 0 ; pos < size ; pos++)
    {
        vect[pos] = pos+1;
    }
}

double vSum(int *vect){

    double sum=0;

    for(int pos = 0 ; pos < size ; pos++)
    {
        printf("%d ", vect[pos]);
    }

    for(int pos=0; pos <size ; pos++){
        sum=sum+vect[pos];
    }
    printf("Sum is equal to: %lf", sum);
    return sum;
}