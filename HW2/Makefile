CC = gcc

CFLAGS = -Og -Wall -Wextra -std=c11

TARGETS = gettimings calculate_timings

GETTIMINGS_SRC = gettimings.c
CALC_SRC = calculate_timings.c

all: $(TARGETS)

gettimings: $(GETTIMINGS_SRC)
	$(CC) $(CFLAGS) -o $@ $^

calculate_timings: $(CALC_SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)