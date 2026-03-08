/*
 * quiz1_q1_tests.c - Shared test implementations for 2018q1 quiz4 Q1.
 *
 * All test functions call through global function pointers (impl_FuncB,
 * impl_FuncC, impl_bubble_sort), so the same tests work for both the
 * original and suggested implementations.
 */

#include "unity.h"
#include "quiz1_q1_tests.h"

funcb_t impl_FuncB;
funcc_t impl_FuncC;
sort_t impl_bubble_sort;

/* ---- FuncB tests ---- */

void test_FuncB_empty_list(void)
{
    struct node *head = NULL;
    impl_FuncB(&head, 42);

    TEST_ASSERT_NOT_NULL(head);
    TEST_ASSERT_EQUAL_INT(42, head->data);
    TEST_ASSERT_EQUAL_PTR(head, head->next);
    TEST_ASSERT_EQUAL_PTR(head, head->prev);
    free_list(head);
}

void test_FuncB_prepend(void)
{
    int vals[] = {1, 2, 3};
    struct node *head = build_list(vals, 3);
    impl_FuncB(&head, 0);

    int arr[4];
    int len = list_to_array(head, arr, 4);

    TEST_ASSERT_EQUAL_INT(4, len);
    TEST_ASSERT_EQUAL_INT(0, arr[0]);
    TEST_ASSERT_EQUAL_INT(1, arr[1]);
    TEST_ASSERT_EQUAL_INT(2, arr[2]);
    TEST_ASSERT_EQUAL_INT(3, arr[3]);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}

/* ---- FuncC tests ---- */

void test_FuncC_empty_list(void)
{
    struct node *head = NULL;
    impl_FuncC(&head, 1, 2);
    TEST_ASSERT_NULL(head);
}

void test_FuncC_value_not_found(void)
{
    int vals[] = {1, 2, 3};
    struct node *head = build_list(vals, 3);

    impl_FuncC(&head, 99, 999);

    int arr[4];
    int len = list_to_array(head, arr, 4);
    TEST_ASSERT_EQUAL_INT(3, len);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}

void test_FuncC_insert_after(void)
{
    int vals[] = {1, 2, 3};
    struct node *head = build_list(vals, 3);

    impl_FuncC(&head, 99, 2);

    int arr[4];
    int len = list_to_array(head, arr, 4);

    TEST_ASSERT_EQUAL_INT(4, len);
    TEST_ASSERT_EQUAL_INT(1, arr[0]);
    TEST_ASSERT_EQUAL_INT(2, arr[1]);
    TEST_ASSERT_EQUAL_INT(99, arr[2]);
    TEST_ASSERT_EQUAL_INT(3, arr[3]);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}

/* ---- bubble_sort tests ---- */

void test_bubble_sort_basic(void)
{
    int vals[] = {4, 2, 3, 1};
    struct node *head = build_list(vals, 4);

    impl_bubble_sort(&head, 4);

    int arr[4];
    list_to_array(head, arr, 4);

    TEST_ASSERT_EQUAL_INT(1, arr[0]);
    TEST_ASSERT_EQUAL_INT(2, arr[1]);
    TEST_ASSERT_EQUAL_INT(3, arr[2]);
    TEST_ASSERT_EQUAL_INT(4, arr[3]);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}

void test_bubble_sort_already_sorted(void)
{
    int vals[] = {1, 2, 3, 4, 5};
    struct node *head = build_list(vals, 5);

    impl_bubble_sort(&head, 5);

    int arr[5];
    list_to_array(head, arr, 5);

    for (int i = 0; i < 5; i++)
        TEST_ASSERT_EQUAL_INT(i + 1, arr[i]);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}

void test_bubble_sort_reverse(void)
{
    int vals[] = {5, 4, 3, 2, 1};
    struct node *head = build_list(vals, 5);

    impl_bubble_sort(&head, 5);

    int arr[5];
    list_to_array(head, arr, 5);

    for (int i = 0; i < 5; i++)
        TEST_ASSERT_EQUAL_INT(i + 1, arr[i]);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}

void test_bubble_sort_single(void)
{
    int vals[] = {42};
    struct node *head = build_list(vals, 1);

    impl_bubble_sort(&head, 1);

    TEST_ASSERT_EQUAL_INT(42, head->data);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}

void test_bubble_sort_duplicates(void)
{
    int vals[] = {3, 1, 2, 1, 3};
    struct node *head = build_list(vals, 5);

    impl_bubble_sort(&head, 5);

    int arr[5];
    list_to_array(head, arr, 5);
    int expected[] = {1, 1, 2, 3, 3};

    for (int i = 0; i < 5; i++)
        TEST_ASSERT_EQUAL_INT(expected[i], arr[i]);
    TEST_ASSERT_TRUE(verify_integrity(head));
    free_list(head);
}
