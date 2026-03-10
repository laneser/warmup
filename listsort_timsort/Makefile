CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=gnu11
LDFLAGS =

SRCS    = bench.c list_sort_vanilla.c list_sort_runs.c
OBJS    = $(SRCS:.c=.o)
TARGET  = bench

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c list.h list_sort.h
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET) 30 64 > results.csv
	@echo "Results saved to results.csv"

clean:
	rm -f $(OBJS) $(TARGET) results.csv
