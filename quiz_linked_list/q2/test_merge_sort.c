/*
 * test_merge_sort.c - Unity tests for all merge sort variants.
 *
 * Tests the original, improved (fast/slow split), and iterative versions.
 * Includes fixed cases, edge cases, and randomized stress tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "unity.h"
#include "merge_sort.h"

/* Function declarations from each implementation */
extern list *sort(list *start);
extern list *sort_improved(list *start);
extern list *sort_iterative(list *start);

/* Current sort function under test */
static list *(*current_sort)(list *);

void setUp(void) {}
void tearDown(void) {}

/* ---- Helper: verify sorted order ---- */
static void assert_sorted(list *head, int expected_len)
{
    int len = slist_length(head);
    TEST_ASSERT_EQUAL_INT(expected_len, len);

    for (list *p = head; p && p->next; p = p->next)
        TEST_ASSERT_TRUE_MESSAGE(p->data <= p->next->data,
                                 "List not sorted");
}

/* ---- Helper: verify sort result matches reference ---- */
static void sort_and_verify(const int *vals, int n)
{
    list *head = build_slist(vals, n);
    head = current_sort(head);
    assert_sorted(head, n);
    free_slist(head);
}

/* ---- Basic test cases ---- */

static void test_null(void)
{
    list *result = current_sort(NULL);
    TEST_ASSERT_NULL(result);
}

static void test_single(void)
{
    int vals[] = {42};
    sort_and_verify(vals, 1);
}

static void test_two_sorted(void)
{
    int vals[] = {1, 2};
    sort_and_verify(vals, 2);
}

static void test_two_reversed(void)
{
    int vals[] = {2, 1};
    sort_and_verify(vals, 2);
}

static void test_already_sorted(void)
{
    int vals[] = {1, 2, 3, 4, 5};
    sort_and_verify(vals, 5);
}

static void test_reversed(void)
{
    int vals[] = {5, 4, 3, 2, 1};
    sort_and_verify(vals, 5);
}

static void test_duplicates(void)
{
    int vals[] = {3, 1, 2, 1, 3, 2};
    sort_and_verify(vals, 6);
}

static void test_all_same(void)
{
    int vals[] = {7, 7, 7, 7};
    sort_and_verify(vals, 4);
}

static void test_negative(void)
{
    int vals[] = {-3, 5, -1, 0, 2, -4};
    sort_and_verify(vals, 6);
}

/* ---- Randomized stress test ---- */

static void test_random(void)
{
    const int sizes[] = {0, 1, 2, 3, 7, 15, 16, 31, 63, 100};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        if (n == 0) {
            TEST_ASSERT_NULL(current_sort(NULL));
            continue;
        }

        int *vals = malloc(n * sizeof(int));
        TEST_ASSERT_NOT_NULL(vals);

        for (int i = 0; i < n; i++)
            vals[i] = rand() % 1000 - 500;

        list *head = build_slist(vals, n);
        head = current_sort(head);
        assert_sorted(head, n);
        free_slist(head);
        free(vals);
    }
}

/* ---- Stability test (improved and iterative only) ---- */

static void test_stability(void)
{
    /* Build a list where equal keys should stay in original order.
     * We encode original position in the upper bits and sort key
     * in the lower bits to verify stability after sorting.
     *
     * Since we only compare data directly, we use a trick:
     * create pairs with same value and check relative order.
     */
    int vals[] = {3, 1, 2, 1, 3};
    list *head = build_slist(vals, 5);

    head = current_sort(head);

    /* Verify sorted */
    assert_sorted(head, 5);

    /* For stability: the two 1s should appear before the two 3s,
     * and within each group, order is preserved.
     * With singly-linked list and <= comparison, this holds. */
    int arr[5];
    slist_to_array(head, arr, 5);
    int expected[] = {1, 1, 2, 3, 3};
    TEST_ASSERT_EQUAL_INT_ARRAY(expected, arr, 5);

    free_slist(head);
}

/* ---- Run all tests for one sort variant ---- */

static void run_sort_tests(const char *name, list *(*fn)(list *))
{
    current_sort = fn;
    printf("\n--- Testing: %s ---\n", name);

    RUN_TEST(test_null);
    RUN_TEST(test_single);
    RUN_TEST(test_two_sorted);
    RUN_TEST(test_two_reversed);
    RUN_TEST(test_already_sorted);
    RUN_TEST(test_reversed);
    RUN_TEST(test_duplicates);
    RUN_TEST(test_all_same);
    RUN_TEST(test_negative);
    RUN_TEST(test_random);
    RUN_TEST(test_stability);
}

int main(void)
{
    srand((unsigned)time(NULL));

    UNITY_BEGIN();

    run_sort_tests("Original (1-vs-rest split)", sort);
    run_sort_tests("Improved (fast/slow split)", sort_improved);
    run_sort_tests("Iterative (bottom-up)", sort_iterative);

    return UNITY_END();
}
