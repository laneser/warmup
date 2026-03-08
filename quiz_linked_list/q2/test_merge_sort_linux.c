/*
 * test_merge_sort_linux.c - Unity tests for list.h-style merge sort.
 *
 * Tests extension 3 (circular doubly-linked list) and 4 (list.h API).
 */

#include <stdlib.h>
#include <time.h>
#include "unity.h"
#include "list.h"

/* From merge_sort_linux.c */
struct listitem {
    int data;
    struct list_head list;
};

typedef int (*cmp_func_t)(const struct list_head *a, const struct list_head *b);

extern void list_mergesort(struct list_head *head, cmp_func_t cmp);

/* Comparison function: ascending order */
static int cmp_int(const struct list_head *a, const struct list_head *b)
{
    int da = list_entry(a, struct listitem, list)->data;
    int db = list_entry(b, struct listitem, list)->data;
    return da - db;
}

void setUp(void) {}
void tearDown(void) {}

/* ---- Helpers ---- */

static void build_list(struct list_head *head, const int *vals, int n)
{
    INIT_LIST_HEAD(head);
    for (int i = 0; i < n; i++) {
        struct listitem *item = malloc(sizeof(*item));
        item->data = vals[i];
        list_add_tail(&item->list, head);
    }
}

static void free_items(struct list_head *head)
{
    struct listitem *item, *tmp;
    list_for_each_entry_safe(item, tmp, head, list) {
        list_del(&item->list);
        free(item);
    }
}

/* Verify circular doubly-linked list integrity */
static void assert_integrity(struct list_head *head)
{
    struct list_head *pos;
    list_for_each(pos, head) {
        TEST_ASSERT_EQUAL_PTR_MESSAGE(pos, pos->next->prev,
                                      "next->prev broken");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(pos, pos->prev->next,
                                      "prev->next broken");
    }
    /* Check head itself */
    TEST_ASSERT_EQUAL_PTR(head, head->next->prev);
    TEST_ASSERT_EQUAL_PTR(head, head->prev->next);
}

/* Verify sorted order and return length */
static int assert_sorted(struct list_head *head)
{
    int count = 0;
    int prev_val = 0;
    int first = 1;
    struct listitem *item;

    list_for_each_entry(item, head, list) {
        if (!first)
            TEST_ASSERT_TRUE_MESSAGE(prev_val <= item->data,
                                     "List not sorted");
        prev_val = item->data;
        first = 0;
        count++;
    }
    return count;
}

static void sort_and_verify(const int *vals, int n)
{
    LIST_HEAD(head);
    build_list(&head, vals, n);

    list_mergesort(&head, cmp_int);

    assert_integrity(&head);
    int len = assert_sorted(&head);
    TEST_ASSERT_EQUAL_INT(n, len);

    free_items(&head);
}

/* ---- Test cases ---- */

void test_empty(void)
{
    LIST_HEAD(head);
    list_mergesort(&head, cmp_int);
    TEST_ASSERT_TRUE(list_empty(&head));
}

void test_single(void)
{
    int vals[] = {42};
    sort_and_verify(vals, 1);
}

void test_two_sorted(void)
{
    int vals[] = {1, 2};
    sort_and_verify(vals, 2);
}

void test_two_reversed(void)
{
    int vals[] = {2, 1};
    sort_and_verify(vals, 2);
}

void test_already_sorted(void)
{
    int vals[] = {1, 2, 3, 4, 5};
    sort_and_verify(vals, 5);
}

void test_reversed(void)
{
    int vals[] = {5, 4, 3, 2, 1};
    sort_and_verify(vals, 5);
}

void test_duplicates(void)
{
    int vals[] = {3, 1, 2, 1, 3, 2};
    sort_and_verify(vals, 6);
}

void test_all_same(void)
{
    int vals[] = {7, 7, 7, 7};
    sort_and_verify(vals, 4);
}

void test_negative(void)
{
    int vals[] = {-3, 5, -1, 0, 2, -4};
    sort_and_verify(vals, 6);
}

void test_random(void)
{
    const int sizes[] = {0, 1, 2, 3, 7, 15, 16, 31, 63, 100};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        LIST_HEAD(head);

        for (int i = 0; i < n; i++) {
            struct listitem *item = malloc(sizeof(*item));
            item->data = rand() % 1000 - 500;
            list_add_tail(&item->list, &head);
        }

        list_mergesort(&head, cmp_int);

        assert_integrity(&head);
        int len = assert_sorted(&head);
        TEST_ASSERT_EQUAL_INT(n, len);

        free_items(&head);
    }
}

/*
 * Compare only the lower 16 bits (sort key), ignoring upper bits (sequence).
 * This lets us verify stability: after sorting, elements with the same key
 * should retain their original relative order (encoded in upper bits).
 */
static int cmp_key_only(const struct list_head *a, const struct list_head *b)
{
    int ka = list_entry(a, struct listitem, list)->data & 0xFFFF;
    int kb = list_entry(b, struct listitem, list)->data & 0xFFFF;
    return ka - kb;
}

void test_stability(void)
{
    /*
     * Pack data = (original_index << 16) | sort_key.
     * Sort by key only. After sorting, for elements with the same key,
     * the original_index (upper bits) must be in ascending order.
     *
     * Input: index=0 key=3, index=1 key=1, index=2 key=2,
     *        index=3 key=1, index=4 key=3
     * Stable sort by key: (1,key=1), (3,key=1), (2,key=2), (0,key=3), (4,key=3)
     * Indices within each key group must be ascending: [1,3], [2], [0,4]
     */
    LIST_HEAD(head);

    int keys[] = {3, 1, 2, 1, 3};
    int n = sizeof(keys) / sizeof(keys[0]);

    for (int i = 0; i < n; i++) {
        struct listitem *item = malloc(sizeof(*item));
        item->data = (i << 16) | keys[i];
        list_add_tail(&item->list, &head);
    }

    list_mergesort(&head, cmp_key_only);

    assert_integrity(&head);

    /* Expected order: (1<<16|1), (3<<16|1), (2<<16|2), (0<<16|3), (4<<16|3) */
    int expected[] = {
        (1 << 16) | 1,
        (3 << 16) | 1,
        (2 << 16) | 2,
        (0 << 16) | 3,
        (4 << 16) | 3,
    };

    int i = 0;
    struct listitem *item;
    list_for_each_entry(item, &head, list) {
        TEST_ASSERT_EQUAL_HEX32_MESSAGE(expected[i], item->data,
                                        "Stability violation");
        i++;
    }
    TEST_ASSERT_EQUAL_INT(n, i);

    free_items(&head);
}

int main(void)
{
    srand((unsigned)time(NULL));

    UNITY_BEGIN();

    RUN_TEST(test_empty);
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

    return UNITY_END();
}
