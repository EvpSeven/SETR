# My first makefile

all: test

test: main.o 
	gcc -o test main.o 

main.o: main.c 
	gcc -o main.o main.c -c -W -Wall
	
clean:
	rm -rf *.o *~ test
