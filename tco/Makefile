CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11

UNITY_DIR = ../quiz_linked_list/unity
UNITY_SRC = $(UNITY_DIR)/unity.c
UNITY_URL = https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src

IMPL_SRC = qsort_impl.c

.PHONY: all clean asm bench csv test plot

all: test bench

# Auto-download Unity if missing
$(UNITY_DIR)/unity.c $(UNITY_DIR)/unity.h $(UNITY_DIR)/unity_internals.h:
	@mkdir -p $(UNITY_DIR)
	curl -sL $(UNITY_URL)/unity.c -o $(UNITY_DIR)/unity.c
	curl -sL $(UNITY_URL)/unity.h -o $(UNITY_DIR)/unity.h
	curl -sL $(UNITY_URL)/unity_internals.h -o $(UNITY_DIR)/unity_internals.h
	@echo "Unity framework downloaded."

# Generate assembly for comparison
asm: qsort_impl_O2.s

qsort_impl_O2.s: $(IMPL_SRC) qsort.h
	$(CC) -O2 -S -fverbose-asm $< -o $@

# Build and run the benchmark (human-readable table)
bench: qsort_bench_O3
	./qsort_bench_O3

qsort_bench_O0: qsort_bench.c $(IMPL_SRC) qsort.h
	$(CC) $(CFLAGS) -O0 qsort_bench.c $(IMPL_SRC) -o $@

qsort_bench_O3: qsort_bench.c $(IMPL_SRC) qsort.h
	$(CC) $(CFLAGS) -O3 qsort_bench.c $(IMPL_SRC) -o $@

# Generate CSV and plot
csv: qsort_bench_O0 qsort_bench_O3
	./qsort_bench_O0 --csv qsort_O0.csv
	./qsort_bench_O3 --csv qsort_O3.csv

plot: csv
	cd .. && uv run plot_bench.py --qsort tco/qsort_O0.csv tco/qsort_O3.csv tco

# Unity tests
test: test_qsort
	./test_qsort

test_qsort: test_qsort.c $(IMPL_SRC) qsort.h $(UNITY_SRC) | $(UNITY_DIR)/unity.h
	$(CC) $(CFLAGS) -I$(UNITY_DIR) test_qsort.c $(IMPL_SRC) $(UNITY_SRC) -o $@

clean:
	rm -f *.s *.o qsort_bench_O0 qsort_bench_O3 test_qsort *.csv *.svg
