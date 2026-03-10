// SPDX-License-Identifier: GPL-2.0
/*
 * Userspace port of list_sort with natural run detection.
 * Based on kp_timsort_sort.c — same merge scheduling as vanilla,
 * but pushes entire natural runs instead of single elements.
 */

#include "list_sort.h"
#include <stdint.h>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static struct list_head *merge(void *priv, list_cmp_func_t cmp,
			       struct list_head *a, struct list_head *b)
{
	struct list_head *head, **tail = &head;

	for (;;) {
		if (cmp(priv, a, b) <= 0) {
			*tail = a;
			tail = &a->next;
			a = a->next;
			if (!a) {
				*tail = b;
				break;
			}
		} else {
			*tail = b;
			tail = &b->next;
			b = b->next;
			if (!b) {
				*tail = a;
				break;
			}
		}
	}
	return head;
}

static void merge_final(void *priv, list_cmp_func_t cmp,
			struct list_head *head,
			struct list_head *a, struct list_head *b)
{
	struct list_head *tail = head;
	uint8_t count = 0;

	for (;;) {
		if (cmp(priv, a, b) <= 0) {
			tail->next = a;
			a->prev = tail;
			tail = a;
			a = a->next;
			if (!a)
				break;
		} else {
			tail->next = b;
			b->prev = tail;
			tail = b;
			b = b->next;
			if (!b) {
				b = a;
				break;
			}
		}
	}

	tail->next = b;
	do {
		if (unlikely(!++count))
			cmp(priv, b, b);
		b->prev = tail;
		tail = b;
		b = b->next;
	} while (b);

	tail->next = head;
	head->prev = tail;
}

void list_sort_runs(void *priv, struct list_head *head, list_cmp_func_t cmp)
{
	struct list_head *list = head->next, *pending = NULL;
	size_t count = 0;

	if (list == head->prev)
		return;

	head->prev->next = NULL;

	do {
		size_t bits;
		struct list_head **tail = &pending;
		struct list_head *run_tail;
		struct list_head *next_list;

		/* Same merge scheduling as vanilla */
		for (bits = count; bits & 1; bits >>= 1)
			tail = &(*tail)->prev;
		if (likely(bits)) {
			struct list_head *a = *tail, *b = a->prev;

			a = merge(priv, cmp, b, a);
			a->prev = b->prev;
			*tail = a;
		}

		/*
		 * Natural run detection: scan for a maximal monotonic
		 * subsequence.  Descending uses strict inequality to
		 * preserve sort stability after reversal.
		 */
		run_tail = list;
		if (run_tail->next &&
		    cmp(priv, run_tail, run_tail->next) > 0) {
			/* Strictly descending run */
			while (run_tail->next &&
			       cmp(priv, run_tail, run_tail->next) > 0)
				run_tail = run_tail->next;

			/* Snip and reverse */
			next_list = run_tail->next;
			run_tail->next = NULL;

			struct list_head *prev = NULL, *curr = list, *tmp;

			while (curr) {
				tmp = curr->next;
				curr->next = prev;
				prev = curr;
				curr = tmp;
			}
			list = prev; /* reversed head */
		} else {
			/* Ascending run (includes single elements) */
			while (run_tail->next &&
			       cmp(priv, run_tail, run_tail->next) <= 0)
				run_tail = run_tail->next;

			next_list = run_tail->next;
			run_tail->next = NULL;
		}

		/* Push this run onto the pending stack */
		list->prev = pending;
		pending = list;
		list = next_list;
		count++;
	} while (list);

	/* Merge all pending lists */
	list = pending;
	pending = pending->prev;
	if (!pending) {
		/* Single run — restore doubly-linked list directly */
		struct list_head *tail = head;

		while (list) {
			tail->next = list;
			list->prev = tail;
			tail = list;
			list = list->next;
		}
		tail->next = head;
		head->prev = tail;
		return;
	}
	for (;;) {
		struct list_head *next = pending->prev;

		if (!next)
			break;
		list = merge(priv, cmp, pending, list);
		pending = next;
	}
	merge_final(priv, cmp, head, pending, list);
}
