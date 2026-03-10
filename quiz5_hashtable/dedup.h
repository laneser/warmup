/*
 * Remove Duplicates from Sorted List II — circular doubly-linked list versions
 *
 * Bug reproductions from qwe661234's solution and corrected versions.
 */
#ifndef DEDUP_H
#define DEDUP_H

#include "list.h"
#include <stdlib.h>

typedef struct {
	int value;
	struct list_head list;
} element_t;

/*
 * Track all allocated elements so we can free them after dedup
 * (dedup removes nodes from the list but doesn't free them).
 */
#define MAX_ELEMENTS 64
static element_t *all_elements[MAX_ELEMENTS];
static int all_elements_count;

/* Helper: build sorted list from array */
static inline void build_list(struct list_head *head, const int *vals, int n)
{
	INIT_LIST_HEAD(head);
	all_elements_count = n;
	for (int i = 0; i < n; i++) {
		element_t *e = malloc(sizeof(*e));
		e->value = vals[i];
		list_add_tail(&e->list, head);
		all_elements[i] = e;
	}
}

/* Helper: convert list to array for comparison, returns count */
static inline int list_to_array(struct list_head *head, int *out, int max)
{
	int cnt = 0;
	struct list_head *pos;
	list_for_each(pos, head) {
		if (cnt >= max)
			break;
		out[cnt++] = list_entry(pos, element_t, list)->value;
	}
	return cnt;
}

/* Helper: free all elements (including those removed from list) */
static inline void free_all_elements(void)
{
	for (int i = 0; i < all_elements_count; i++)
		free(all_elements[i]);
	all_elements_count = 0;
}

/* ---- Original iterative version (qwe661234) ---- */
static inline void dedup_iter_original(struct list_head *head)
{
	if (!head)
		return;
	struct list_head *cur, *safe;
	list_for_each_safe(cur, safe, head) {
		/* BUG: no check for safe != head before container_of */
		if (list_entry(cur, element_t, list)->value ==
		    list_entry(safe, element_t, list)->value) {
			while (safe != head &&
			       list_entry(cur, element_t, list)->value ==
				   list_entry(safe, element_t, list)->value) {
				list_del(cur);
				cur = safe;
				safe = safe->next;
			}
			list_del(cur);
		}
	}
}

/* ---- Fixed iterative version ---- */
static inline void dedup_iter_fixed(struct list_head *head)
{
	if (!head)
		return;
	struct list_head *cur, *safe;
	list_for_each_safe(cur, safe, head) {
		if (safe != head &&
		    list_entry(cur, element_t, list)->value ==
			list_entry(safe, element_t, list)->value) {
			while (safe != head &&
			       list_entry(cur, element_t, list)->value ==
				   list_entry(safe, element_t, list)->value) {
				list_del(cur);
				cur = safe;
				safe = safe->next;
			}
			list_del(cur);
		}
	}
}

/* ---- Original recursive version (qwe661234) ---- */
static inline void dedup_recur_original(struct list_head *cur,
				     struct list_head *head)
{
	if (cur == head)
		return;
	if ((cur->next != head &&
	     list_entry(cur, element_t, list)->value ==
		 list_entry(cur->next, element_t, list)->value) ||
	    (cur->prev != head &&
	     list_entry(cur, element_t, list)->value ==
		 list_entry(cur->prev, element_t, list)->value)) {
		dedup_recur_original(cur->next, head);
		list_del(cur);
	}
	/* BUG: no else — always recurses again, even after list_del */
	dedup_recur_original(cur->next, head);
}

/* ---- Fixed recursive version ---- */
static inline void dedup_recur_fixed(struct list_head *cur,
				     struct list_head *head)
{
	if (cur == head)
		return;
	struct list_head *next = cur->next;
	if ((next != head &&
	     list_entry(cur, element_t, list)->value ==
		 list_entry(next, element_t, list)->value) ||
	    (cur->prev != head &&
	     list_entry(cur, element_t, list)->value ==
		 list_entry(cur->prev, element_t, list)->value)) {
		dedup_recur_fixed(next, head);
		list_del(cur);
	} else {
		dedup_recur_fixed(next, head);
	}
}

#endif /* DEDUP_H */
