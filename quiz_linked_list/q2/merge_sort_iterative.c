/*
 * merge_sort_iterative.c - Bottom-up iterative merge sort.
 *
 * Avoids recursion entirely. Merges runs of size 1, 2, 4, 8, ...
 * Uses O(1) extra space (no call stack).
 */

#include "merge_sort.h"

/* Merge two sorted lists. */
static list *merge(list *a, list *b)
{
    list dummy;
    list *tail = &dummy;
    dummy.next = NULL;

    while (a && b) {
        if (a->data <= b->data) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a ? a : b;

    return dummy.next;
}

/*
 * Split off a run of up to `count` nodes from *head.
 * Updates *head to point past the split portion.
 * Returns the split-off sublist.
 */
static list *split_at(list **head, int count)
{
    list *run = *head;
    list *prev = NULL;

    for (int i = 0; i < count && *head; i++) {
        prev = *head;
        *head = (*head)->next;
    }
    if (prev)
        prev->next = NULL;

    return run;
}

list *sort_iterative(list *start)
{
    if (!start || !start->next)
        return start;

    int len = slist_length(start);

    for (int width = 1; width < len; width *= 2) {
        list dummy;
        dummy.next = NULL;
        list *tail = &dummy;
        list *cur = start;

        while (cur) {
            list *left = split_at(&cur, width);
            list *right = split_at(&cur, width);
            list *merged = merge(left, right);

            tail->next = merged;
            while (tail->next)
                tail = tail->next;
        }
        start = dummy.next;
    }

    return start;
}
