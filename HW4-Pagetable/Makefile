CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2 

.PHONY: clean

all: libmlpt.a

libmlpt.a: mlpt.o
	ar rcs $@ $^

mlpt.o: mlpt.c mlpt.h config.h
	$(CC) $(CFLAGS) -c mlpt.c -o mlpt.o

clean:
	rm -f *.o libmlpt.a