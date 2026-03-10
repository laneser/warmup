/*
 * test_list_sort.c - Unity tests for list_sort_runs correctness.
 *
 * Verifies sorting, list integrity, stability, and edge cases.
 * Also runs list_sort_vanilla as a reference to ensure both produce
 * identical results.
 */

#include <stdlib.h>
#include <time.h>
#include "unity.h"
#include "list_sort.h"

struct listitem {
	int data;
	struct list_head list;
};

/* Comparison: ascending order by data. priv is unused. */
static int cmp_int(void *priv, const struct list_head *a,
		   const struct list_head *b)
{
	(void)priv;
	int da = list_entry(a, struct listitem, list)->data;
	int db = list_entry(b, struct listitem, list)->data;
	return da - db;
}

/*
 * Compare lower 16 bits only (sort key), ignoring upper bits (sequence).
 * Used to verify stability.
 */
static int cmp_key_only(void *priv, const struct list_head *a,
			const struct list_head *b)
{
	(void)priv;
	int ka = list_entry(a, struct listitem, list)->data & 0xFFFF;
	int kb = list_entry(b, struct listitem, list)->data & 0xFFFF;
	return ka - kb;
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

static void free_list(struct list_head *head)
{
	struct list_head *pos, *tmp;

	list_for_each_safe(pos, tmp, head) {
		struct listitem *item = list_entry(pos, struct listitem, list);
		list_del(pos);
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
	TEST_ASSERT_EQUAL_PTR(head, head->next->prev);
	TEST_ASSERT_EQUAL_PTR(head, head->prev->next);
}

/* Verify sorted order and return length */
static int assert_sorted(struct list_head *head)
{
	int count = 0, prev_val = 0, first = 1;
	struct list_head *pos;

	list_for_each(pos, head) {
		int val = list_entry(pos, struct listitem, list)->data;
		if (!first)
			TEST_ASSERT_TRUE_MESSAGE(prev_val <= val,
						 "List not sorted");
		prev_val = val;
		first = 0;
		count++;
	}
	return count;
}

/* Sort with list_sort_runs and verify correctness */
static void sort_and_verify(const int *vals, int n)
{
	LIST_HEAD(head);

	build_list(&head, vals, n);
	list_sort_runs(NULL, &head, cmp_int);
	assert_integrity(&head);
	int len = assert_sorted(&head);
	TEST_ASSERT_EQUAL_INT(n, len);
	free_list(&head);
}

/* ---- Test cases ---- */

void test_empty(void)
{
	LIST_HEAD(head);

	list_sort_runs(NULL, &head, cmp_int);
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
	int vals[] = {1, 2, 3, 4, 5, 6, 7, 8};

	sort_and_verify(vals, 8);
}

void test_reversed(void)
{
	int vals[] = {8, 7, 6, 5, 4, 3, 2, 1};

	sort_and_verify(vals, 8);
}

void test_duplicates(void)
{
	int vals[] = {3, 1, 2, 1, 3, 2};

	sort_and_verify(vals, 6);
}

void test_all_same(void)
{
	int vals[] = {7, 7, 7, 7, 7};

	sort_and_verify(vals, 5);
}

void test_negative(void)
{
	int vals[] = {-3, 5, -1, 0, 2, -4};

	sort_and_verify(vals, 6);
}

/* Ascending runs followed by descending run */
void test_mixed_runs(void)
{
	int vals[] = {1, 3, 5, 7, 9, 8, 6, 4, 2, 0};

	sort_and_verify(vals, 10);
}

/* Pipe organ: ascending then descending */
void test_pipe_organ(void)
{
	int vals[] = {1, 3, 5, 7, 9, 10, 8, 6, 4, 2};

	sort_and_verify(vals, 10);
}

/* Multiple short ascending runs */
void test_short_asc_runs(void)
{
	int vals[] = {3, 4, 5, 1, 2, 3, 7, 8, 9, 0, 1, 2};

	sort_and_verify(vals, 12);
}

/* Multiple short descending runs */
void test_short_desc_runs(void)
{
	int vals[] = {5, 4, 3, 9, 8, 7, 2, 1, 0, 6, 5, 4};

	sort_and_verify(vals, 12);
}

void test_random_sizes(void)
{
	const int sizes[] = {0, 1, 2, 3, 7, 15, 16, 31, 63, 64, 100, 255, 1000};

	for (int s = 0; s < (int)(sizeof(sizes) / sizeof(sizes[0])); s++) {
		int n = sizes[s];
		LIST_HEAD(head);

		for (int i = 0; i < n; i++) {
			struct listitem *item = malloc(sizeof(*item));
			item->data = rand() % 2000 - 1000;
			list_add_tail(&item->list, &head);
		}

		list_sort_runs(NULL, &head, cmp_int);
		assert_integrity(&head);
		int len = assert_sorted(&head);
		TEST_ASSERT_EQUAL_INT(n, len);
		free_list(&head);
	}
}

void test_stability(void)
{
	/*
	 * Pack data = (original_index << 16) | sort_key.
	 * Sort by key only. After sorting, for elements with the same key,
	 * the original_index (upper bits) must be in ascending order.
	 */
	LIST_HEAD(head);
	int keys[] = {3, 1, 2, 1, 3, 2, 1};
	int n = sizeof(keys) / sizeof(keys[0]);

	for (int i = 0; i < n; i++) {
		struct listitem *item = malloc(sizeof(*item));
		item->data = (i << 16) | keys[i];
		list_add_tail(&item->list, &head);
	}

	list_sort_runs(NULL, &head, cmp_key_only);
	assert_integrity(&head);

	/* Verify: within each key group, indices must be ascending */
	int prev_key = -1, prev_idx = -1;
	struct list_head *pos;

	list_for_each(pos, &head) {
		int val = list_entry(pos, struct listitem, list)->data;
		int key = val & 0xFFFF;
		int idx = val >> 16;

		TEST_ASSERT_TRUE_MESSAGE(key >= prev_key,
					 "Keys not sorted");
		if (key == prev_key)
			TEST_ASSERT_TRUE_MESSAGE(idx > prev_idx,
						 "Stability violation");
		prev_key = key;
		prev_idx = idx;
	}

	free_list(&head);
}

/*
 * Cross-check: sort identical input with both vanilla and runs,
 * verify they produce the same output.
 */
void test_matches_vanilla(void)
{
	const int n = 200;

	int *vals = malloc(n * sizeof(int));
	for (int i = 0; i < n; i++)
		vals[i] = rand() % 500 - 250;

	LIST_HEAD(head_v);
	LIST_HEAD(head_r);

	build_list(&head_v, vals, n);
	build_list(&head_r, vals, n);

	list_sort_vanilla(NULL, &head_v, cmp_int);
	list_sort_runs(NULL, &head_r, cmp_int);

	/* Both must produce identical element order */
	struct list_head *pv = head_v.next, *pr = head_r.next;
	int count = 0;

	while (pv != &head_v && pr != &head_r) {
		int dv = list_entry(pv, struct listitem, list)->data;
		int dr = list_entry(pr, struct listitem, list)->data;
		TEST_ASSERT_EQUAL_INT_MESSAGE(dv, dr,
					      "vanilla vs runs mismatch");
		pv = pv->next;
		pr = pr->next;
		count++;
	}
	TEST_ASSERT_EQUAL_INT(n, count);
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&head_v, pv, "vanilla list longer");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&head_r, pr, "runs list longer");

	free_list(&head_v);
	free_list(&head_r);
	free(vals);
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
	RUN_TEST(test_mixed_runs);
	RUN_TEST(test_pipe_organ);
	RUN_TEST(test_short_asc_runs);
	RUN_TEST(test_short_desc_runs);
	RUN_TEST(test_random_sizes);
	RUN_TEST(test_stability);
	RUN_TEST(test_matches_vanilla);

	return UNITY_END();
}
