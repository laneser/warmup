/*
 * Unity tests for quiz5 (2022q1 quiz1) bug reproductions:
 *   1. dedup iterative: sentinel head container_of UB
 *   2. dedup recursive: missing else + use-after-list_del
 *
 * Usage:
 *   ./test_quiz5          — run fixed-version tests only (all should PASS)
 *   ./test_quiz5 original    — run original-version tests (expect FAIL / crash)
 */
#include "unity.h"

#include "dedup.h"

#include <string.h>

static int run_original = 0;

void setUp(void) {}
void tearDown(void) {}

/* ================================================================
 * Helper: run a dedup function and compare result
 * ================================================================ */

typedef void (*dedup_fn)(struct list_head *);
typedef void (*dedup_recur_fn)(struct list_head *, struct list_head *);

static void check_dedup(dedup_fn fn, const int *input, int n,
			const int *expected, int expected_n, const char *label)
{
	struct list_head head;
	build_list(&head, input, n);

	fn(&head);

	int result[64];
	int cnt = list_to_array(&head, result, 64);
	char msg[128];
	snprintf(msg, sizeof(msg), "%s: count mismatch", label);
	TEST_ASSERT_EQUAL_INT_MESSAGE(expected_n, cnt, msg);
	if (expected_n > 0) {
		snprintf(msg, sizeof(msg), "%s: values mismatch", label);
		TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected, result,
						    expected_n, msg);
	}
	free_all_elements();
}

static void check_dedup_recur(dedup_recur_fn fn, const int *input, int n,
			      const int *expected, int expected_n,
			      const char *label)
{
	struct list_head head;
	build_list(&head, input, n);

	fn(head.next, &head);

	int result[64];
	int cnt = list_to_array(&head, result, 64);
	char msg[128];
	snprintf(msg, sizeof(msg), "%s: count mismatch", label);
	TEST_ASSERT_EQUAL_INT_MESSAGE(expected_n, cnt, msg);
	if (expected_n > 0) {
		snprintf(msg, sizeof(msg), "%s: values mismatch", label);
		TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected, result,
						    expected_n, msg);
	}
	free_all_elements();
}

/* ================================================================
 * FIXED versions — should all PASS
 * ================================================================ */

void test_dedup_iter_fixed_last_unique(void)
{
	int input[] = {1, 1, 2};
	int expected[] = {2};
	check_dedup(dedup_iter_fixed, input, 3, expected, 1,
		    "iter_fixed [1,1,2]");
}

void test_dedup_iter_fixed_all_dup(void)
{
	int input[] = {1, 1, 2, 2, 3, 3};
	check_dedup(dedup_iter_fixed, input, 6, NULL, 0,
		    "iter_fixed [1,1,2,2,3,3]");
}

void test_dedup_iter_fixed_single(void)
{
	int input[] = {5};
	int expected[] = {5};
	check_dedup(dedup_iter_fixed, input, 1, expected, 1,
		    "iter_fixed [5]");
}

void test_dedup_iter_fixed_mixed(void)
{
	int input[] = {1, 2, 3, 3, 4, 4, 5};
	int expected[] = {1, 2, 5};
	check_dedup(dedup_iter_fixed, input, 7, expected, 3,
		    "iter_fixed [1,2,3,3,4,4,5]");
}

void test_dedup_iter_fixed_no_dup(void)
{
	int input[] = {1, 2, 3, 4, 5};
	int expected[] = {1, 2, 3, 4, 5};
	check_dedup(dedup_iter_fixed, input, 5, expected, 5,
		    "iter_fixed [1,2,3,4,5]");
}

void test_dedup_recur_fixed_basic(void)
{
	int input[] = {1, 1, 2};
	int expected[] = {2};
	check_dedup_recur(dedup_recur_fixed, input, 3, expected, 1,
			  "recur_fixed [1,1,2]");
}

void test_dedup_recur_fixed_mixed(void)
{
	int input[] = {1, 2, 3, 3, 4, 4, 5};
	int expected[] = {1, 2, 5};
	check_dedup_recur(dedup_recur_fixed, input, 7, expected, 3,
			  "recur_fixed [1,2,3,3,4,4,5]");
}

void test_dedup_recur_fixed_triple(void)
{
	int input[] = {1, 1, 1, 2, 3};
	int expected[] = {2, 3};
	check_dedup_recur(dedup_recur_fixed, input, 5, expected, 2,
			  "recur_fixed [1,1,1,2,3]");
}

void test_dedup_recur_fixed_no_dup(void)
{
	int input[] = {1, 2, 3, 4, 5};
	int expected[] = {1, 2, 3, 4, 5};
	check_dedup_recur(dedup_recur_fixed, input, 5, expected, 5,
			  "recur_fixed [1,2,3,4,5]");
}

/* ================================================================
 * Original versions — demonstrate the issues
 * ================================================================ */

void test_dedup_iter_original_last_unique(void)
{
	int input[] = {1, 1, 2};
	int expected[] = {2};
	check_dedup(dedup_iter_original, input, 3, expected, 1,
		    "iter_original [1,1,2]");
}

void test_dedup_iter_original_mixed(void)
{
	int input[] = {1, 2, 3, 3, 4, 4, 5};
	int expected[] = {1, 2, 5};
	check_dedup(dedup_iter_original, input, 7, expected, 3,
		    "iter_original [1,2,3,3,4,4,5]");
}

void test_dedup_recur_original_basic(void)
{
	int input[] = {1, 1, 2};
	int expected[] = {2};
	check_dedup_recur(dedup_recur_original, input, 3, expected, 1,
			  "recur_original [1,1,2]");
}

void test_dedup_recur_original_mixed(void)
{
	int input[] = {1, 2, 3, 3, 4, 4, 5};
	int expected[] = {1, 2, 5};
	check_dedup_recur(dedup_recur_original, input, 7, expected, 3,
			  "recur_original [1,2,3,3,4,4,5]");
}

/* ================================================================ */

int main(int argc, char *argv[])
{
	if (argc > 1 && strcmp(argv[1], "original") == 0)
		run_original = 1;

	UNITY_BEGIN();

	/* Fixed versions — always run */
	RUN_TEST(test_dedup_iter_fixed_last_unique);
	RUN_TEST(test_dedup_iter_fixed_all_dup);
	RUN_TEST(test_dedup_iter_fixed_single);
	RUN_TEST(test_dedup_iter_fixed_mixed);
	RUN_TEST(test_dedup_iter_fixed_no_dup);
	RUN_TEST(test_dedup_recur_fixed_basic);
	RUN_TEST(test_dedup_recur_fixed_mixed);
	RUN_TEST(test_dedup_recur_fixed_triple);
	RUN_TEST(test_dedup_recur_fixed_no_dup);

	/* Original versions — only with "original" argument */
	if (run_original) {
		/* iter original: UB but may not crash without sanitizer */
		RUN_TEST(test_dedup_iter_original_last_unique);
		RUN_TEST(test_dedup_iter_original_mixed);
		/* recur original: segfault (NULL deref after list_del) */
		RUN_TEST(test_dedup_recur_original_basic);
		RUN_TEST(test_dedup_recur_original_mixed);
	}

	return UNITY_END();
}
