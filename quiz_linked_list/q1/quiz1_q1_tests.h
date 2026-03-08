/*
 * quiz1_q1_tests.h - Shared test declarations for 2018q1 quiz4 Q1.
 *
 * Test functions use global function pointers so the same tests
 * can run against both original and suggested implementations.
 */

#ifndef QUIZ1_Q1_TESTS_H
#define QUIZ1_Q1_TESTS_H

#include "quiz1_q1.h"

/* Function pointer types matching the quiz API */
typedef void (*funcb_t)(struct node **start, int value);
typedef void (*funcc_t)(struct node **start, int value1, int value2);
typedef void (*sort_t)(struct node **start, int length);

/* Global pointers — set by main() before running tests */
extern funcb_t impl_FuncB;
extern funcc_t impl_FuncC;
extern sort_t impl_bubble_sort;

/* Test functions (Unity-compatible void(void) signature) */
void test_FuncB_empty_list(void);
void test_FuncB_prepend(void);
void test_FuncC_empty_list(void);
void test_FuncC_value_not_found(void);
void test_FuncC_insert_after(void);
void test_bubble_sort_basic(void);
void test_bubble_sort_already_sorted(void);
void test_bubble_sort_reverse(void);
void test_bubble_sort_single(void);
void test_bubble_sort_duplicates(void);

#endif /* QUIZ1_Q1_TESTS_H */
