CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

all: split

split: main.o split.o
	$(CC) $(CFLAGS) -o split main.o split.o

main.o: main.c split.h
	$(CC) $(CFLAGS) -c main.c

split.o: split.c split.h
	$(CC) $(CFLAGS) -c split.c

.PHONY: clean
clean:
	rm -f *.o split