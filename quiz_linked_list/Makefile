CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

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

.PHONY: all clean q1_report

all: test_q1 test_q1_bug

clean:
	rm -f test_q1 test_q1_bug
