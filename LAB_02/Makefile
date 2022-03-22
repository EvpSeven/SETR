P = app
OBJECTS = main.o
CFLAGS = -g -Wall
CC = gcc

all: $(P)

# Generate application
$(P): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(P) $(OBJECTS)

# Generate object files
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o $(P)