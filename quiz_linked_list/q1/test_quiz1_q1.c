/*
 * test_quiz1_q1.c - Unity tests for the SUGGESTED 2018q1 quiz4 Q1.
 *
 * Binds suggested implementations to shared test functions and runs
 * them directly via Unity. All 10 tests should PASS.
 */

#include "unity.h"
#include "quiz1_q1_tests.h"

/* Suggested implementations (defined in fix_quiz1_q1.c) */
extern void FuncB(struct node **start, int value);
extern void FuncC(struct node **start, int value1, int value2);
extern void bubble_sort(struct node **start, int length);

void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    impl_FuncB = FuncB;
    impl_FuncC = FuncC;
    impl_bubble_sort = bubble_sort;

    UNITY_BEGIN();

    RUN_TEST(test_FuncB_empty_list);
    RUN_TEST(test_FuncB_prepend);
    RUN_TEST(test_FuncC_empty_list);
    RUN_TEST(test_FuncC_value_not_found);
    RUN_TEST(test_FuncC_insert_after);
    RUN_TEST(test_bubble_sort_basic);
    RUN_TEST(test_bubble_sort_already_sorted);
    RUN_TEST(test_bubble_sort_reverse);
    RUN_TEST(test_bubble_sort_single);
    RUN_TEST(test_bubble_sort_duplicates);

    return UNITY_END();
}
