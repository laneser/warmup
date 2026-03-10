CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -g

UNITY_DIR = ../quiz_linked_list/unity
UNITY_SRC = $(UNITY_DIR)/unity.c

.PHONY: all clean test test-asan report

all: test

test_quiz5: test_quiz5.c dedup.h list.h $(UNITY_SRC)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) test_quiz5.c $(UNITY_SRC) -o $@

test_quiz5_asan: test_quiz5.c dedup.h list.h $(UNITY_SRC)
	$(CC) $(CFLAGS) -fsanitize=address -I$(UNITY_DIR) test_quiz5.c $(UNITY_SRC) -o $@

test: test_quiz5
	./test_quiz5

test-asan: test_quiz5_asan
	./test_quiz5_asan

# One-command summary of all findings
report: test_quiz5 test_quiz5_asan
	@echo "=== Fixed versions (9 tests) ==="
	@./test_quiz5
	@echo ""
	@echo "=== Original iterative: ASan detects sentinel head UB ==="
	@./test_quiz5_asan original 2>&1 | grep 'ERROR:\|SUMMARY:' || true
	@echo ""
	@echo "=== Original recursive: segfault (NULL deref after list_del) ==="
	@bash -c './test_quiz5 original >/dev/null 2>&1; echo "exit code: $$? (139 = segfault)"' 2>/dev/null

clean:
	rm -f test_quiz5 test_quiz5_asan
