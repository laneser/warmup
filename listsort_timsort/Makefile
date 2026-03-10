CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=gnu11
LDFLAGS =

UNITY_DIR = unity
UNITY_SRC = $(UNITY_DIR)/unity.c

SRCS    = bench.c list_sort_vanilla.c list_sort_runs.c
OBJS    = $(SRCS:.c=.o)
TARGET  = bench

TEST_TARGET = test_list_sort
TEST_SRCS   = test_list_sort.c list_sort_vanilla.c list_sort_runs.c $(UNITY_SRC)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_TARGET): $(TEST_SRCS) list.h list_sort.h
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -o $@ $(TEST_SRCS) $(LDFLAGS)

%.o: %.c list.h list_sort.h
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TEST_TARGET)
	./$(TEST_TARGET)

run: $(TARGET)
	./$(TARGET) 30 64 > results.csv
	@echo "Results saved to results.csv"

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET) results.csv
