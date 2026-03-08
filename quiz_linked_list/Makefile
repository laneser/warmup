CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra

UNITY_DIR = unity
UNITY_SRC = $(UNITY_DIR)/unity.c
UNITY_URL = https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src

# Auto-download Unity if missing
$(UNITY_DIR)/unity.c $(UNITY_DIR)/unity.h $(UNITY_DIR)/unity_internals.h:
	@mkdir -p $(UNITY_DIR)
	curl -sL $(UNITY_URL)/unity.c -o $(UNITY_DIR)/unity.c
	curl -sL $(UNITY_URL)/unity.h -o $(UNITY_DIR)/unity.h
	curl -sL $(UNITY_URL)/unity_internals.h -o $(UNITY_DIR)/unity_internals.h
	@echo "Unity framework downloaded."

# ---- Q1: 2018q1 quiz4 Q1 ----

Q1_SHARED = q1/quiz1_q1_tests.c

test_q1: q1/test_quiz1_q1.c q1/fix_quiz1_q1.c $(Q1_SHARED) $(UNITY_SRC) | $(UNITY_DIR)/unity.h
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -Iq1 -o $@ $^

test_q1_bug: q1/test_quiz1_q1_bug.c q1/quiz1_q1.c $(Q1_SHARED) $(UNITY_SRC) | $(UNITY_DIR)/unity.h
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -Iq1 -o $@ $^

q1_report: test_q1 test_q1_bug
	@echo ""
	@echo "=== Q1 Suggested version (Unity tests) ==="
	@./test_q1
	@echo ""
	@echo "=== Q1 Original version (expected failures) ==="
	@./test_q1_bug; true

# ---- Q2: 2020q1 quiz1 merge sort ----

Q2_SRC = q2/merge_sort_original.c q2/merge_sort_improved.c q2/merge_sort_iterative.c

test_q2: q2/test_merge_sort.c $(Q2_SRC) $(UNITY_SRC) | $(UNITY_DIR)/unity.h
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -Iq2 -o $@ $^

test_q2_linux: q2/test_merge_sort_linux.c q2/merge_sort_linux.c $(UNITY_SRC) | $(UNITY_DIR)/unity.h
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -Iq2 -o $@ $^

q2_report: test_q2 test_q2_linux
	@echo ""
	@echo "=== Q2 Merge sort singly-linked (Unity tests) ==="
	@./test_q2
	@echo ""
	@echo "=== Q2 Merge sort list.h style (Unity tests) ==="
	@./test_q2_linux

.PHONY: all clean q1_report q2_report

all: test_q1 test_q1_bug test_q2 test_q2_linux

clean:
	rm -f test_q1 test_q1_bug test_q2 test_q2_linux
