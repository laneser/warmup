/*
 * merge_sort_linux.c - Merge sort using Linux kernel list.h style.
 *
 * Extension 3+4: Circular doubly-linked list with list_head and container_of.
 *
 * Strategy (mirrors lib/list_sort.c approach):
 *   1. Break circular DLL into a NULL-terminated singly-linked list
 *   2. Split with fast/slow pointers, merge sort recursively
 *   3. Restore prev links and circularity
 *
 * The comparison function takes two list_head pointers, following the
 * kernel's list_sort() convention (cmp returns <= 0 for a <= b).
 */

#include "list.h"
#include <stdlib.h>

/* Data node: embeds list_head, data is accessed via container_of */
struct listitem {
    int data;
    struct list_head list;
};

typedef int (*cmp_func_t)(const struct list_head *a, const struct list_head *b);

/*
 * Merge two NULL-terminated singly-linked lists (next-only).
 * Stable: equal elements preserve original order.
 */
static struct list_head *merge(cmp_func_t cmp,
                               struct list_head *a,
                               struct list_head *b)
{
    struct list_head *head, **tail = &head;

    for (;;) {
        if (cmp(a, b) <= 0) {
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

/*
 * Recursive merge sort on a NULL-terminated singly-linked list.
 * Uses fast/slow pointer for O(n log n) split.
 */
static struct list_head *merge_sort(cmp_func_t cmp, struct list_head *head)
{
    if (!head || !head->next)
        return head;

    /* Fast/slow pointer split */
    struct list_head *slow = head;
    struct list_head *fast = head->next;
    while (fast) {
        fast = fast->next;
        if (fast) {
            slow = slow->next;
            fast = fast->next;
        }
    }
    struct list_head *mid = slow->next;
    slow->next = NULL;

    struct list_head *left = merge_sort(cmp, head);
    struct list_head *right = merge_sort(cmp, mid);

    return merge(cmp, left, right);
}

/*
 * Sort a circular doubly-linked list in-place.
 *
 * @head: the sentinel head node (not a data node)
 * @cmp:  comparison function on list_head pointers
 */
void list_mergesort(struct list_head *head, cmp_func_t cmp)
{
    if (list_empty(head) || list_is_singular(head))
        return;

    /* Step 1: Break circularity — convert to NULL-terminated */
    struct list_head *first = head->next;
    head->prev->next = NULL;

    /* Step 2: Sort */
    first = merge_sort(cmp, first);

    /* Step 3: Rebuild prev links and re-attach to sentinel */
    head->next = first;
    first->prev = head;

    struct list_head *cur = first;
    while (cur->next) {
        cur->next->prev = cur;
        cur = cur->next;
    }
    /* Close the circle */
    cur->next = head;
    head->prev = cur;
}
