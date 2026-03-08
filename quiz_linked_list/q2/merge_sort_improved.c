/*
 * merge_sort_improved.c - Improved merge sort on singly-linked list.
 *
 * Improvements over the original:
 *   1. Fast/slow pointer split for O(n log n) complexity
 *   2. Merge uses <= for stability (preserves relative order of equal keys)
 *   3. Terminates merged list with NULL (original has dangling next pointers)
 */

#include "merge_sort.h"

/* Split list into two halves using fast/slow pointers. */
static void split(list *head, list **front, list **back)
{
    list *slow = head;
    list *fast = head->next;

    while (fast) {
        fast = fast->next;
        if (fast) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *front = head;
    *back = slow->next;
    slow->next = NULL;
}

/* Merge two sorted lists. Stable: equal elements keep original order. */
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

list *sort_improved(list *start)
{
    if (!start || !start->next)
        return start;

    list *front, *back;
    split(start, &front, &back);

    front = sort_improved(front);
    back = sort_improved(back);

    return merge(front, back);
}
