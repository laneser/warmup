/*
 * test_qsort.c - Unity tests for all four quicksort variants.
 *
 * Tests: empty, single, sorted, reversed, duplicates, all-same,
 *        negative values, and randomized stress tests.
 * Quicksort is NOT stable, so we only verify sorted order and
 * element preservation (no missing/extra elements).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "unity.h"
#include "qsort.h"

/* Current sort function under test */
static qsort_fn current_sort;

void setUp(void) {}
void tearDown(void) {}

/* ---- Helper: compare ints for qsort(3) ---- */
static int cmp_int(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

/* ---- Helper: sort list and verify against reference ---- */
static void sort_and_verify(const int *vals, int n)
{
    node_t *head = make_list(vals, n);

    current_sort(&head);

    /* Verify length and collect values */
    int *got = malloc(n * sizeof(int));
    int len = list_to_array(head, got, n);
    TEST_ASSERT_EQUAL_INT_MESSAGE(n, len, "Length mismatch after sort");

    /* No extra elements beyond n */
    node_t *p = head;
    for (int i = 0; i < n; i++)
        p = p->next;
    TEST_ASSERT_NULL_MESSAGE(p, "List longer than expected");

    /* Build reference: sort input with qsort(3) */
    int *expected = malloc(n * sizeof(int));
    memcpy(expected, vals, n * sizeof(int));
    qsort(expected, n, sizeof(int), cmp_int);

    TEST_ASSERT_EQUAL_INT_ARRAY(expected, got, n);

    free(expected);
    free(got);
    free_list(head);
}

/* ---- Basic test cases ---- */

static void test_empty(void)
{
    node_t *head = NULL;
    current_sort(&head);
    TEST_ASSERT_NULL(head);
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

static void test_mixed_large(void)
{
    int vals[] = {100, -50, 0, 99, -99, 50, 1, -1, 42, -42,
                  77, -77, 33, -33, 10, -10, 25, -25, 88, -88};
    sort_and_verify(vals, 20);
}

/* ---- Worst-case for stack depth: sorted input ---- */

static void test_sorted_100(void)
{
    int vals[100];
    for (int i = 0; i < 100; i++)
        vals[i] = i;
    sort_and_verify(vals, 100);
}

static void test_reversed_100(void)
{
    int vals[100];
    for (int i = 0; i < 100; i++)
        vals[i] = 99 - i;
    sort_and_verify(vals, 100);
}

/* ---- Randomized stress test ---- */

static void test_random_various_sizes(void)
{
    const int sizes[] = {0, 1, 2, 3, 7, 15, 16, 31, 63, 100, 500};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        if (n == 0) {
            node_t *head = NULL;
            current_sort(&head);
            TEST_ASSERT_NULL(head);
            continue;
        }

        int *vals = malloc(n * sizeof(int));
        TEST_ASSERT_NOT_NULL(vals);

        for (int i = 0; i < n; i++)
            vals[i] = rand() % 1000 - 500;

        sort_and_verify(vals, n);
        free(vals);
    }
}

/* ---- Run all tests for one sort variant ---- */

static void run_sort_tests(const char *name, qsort_fn fn)
{
    current_sort = fn;
    printf("\n--- Testing: %s ---\n", name);

    RUN_TEST(test_empty);
    RUN_TEST(test_single);
    RUN_TEST(test_two_sorted);
    RUN_TEST(test_two_reversed);
    RUN_TEST(test_already_sorted);
    RUN_TEST(test_reversed);
    RUN_TEST(test_duplicates);
    RUN_TEST(test_all_same);
    RUN_TEST(test_negative);
    RUN_TEST(test_mixed_large);
    RUN_TEST(test_sorted_100);
    RUN_TEST(test_reversed_100);
    RUN_TEST(test_random_various_sizes);
}

int main(void)
{
    srand((unsigned)time(NULL));

    UNITY_BEGIN();

    run_sort_tests("qsort_original (two recursive calls)", qsort_original);
    run_sort_tests("qsort_tco (linD026: skip empty partitions)", qsort_tco);
    run_sort_tests("qsort_method2 (recurse smaller, loop larger)", qsort_method2);
    run_sort_tests("qsort_iterative (explicit stack, no recursion)", qsort_iterative);

    return UNITY_END();
}
