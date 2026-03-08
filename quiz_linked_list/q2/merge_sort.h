/*
 * merge_sort.h - 2020q1 quiz1: merge sort on singly-linked list.
 *
 * Original quiz: https://hackmd.io/@sysprog/linux2020-quiz1
 * Contains the original (buggy) sort, improved version, and helpers.
 */

#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <stdlib.h>

typedef struct __list {
    int data;
    struct __list *next;
} list;

/* ---- Helpers ---- */

static inline list *build_slist(const int *vals, int n)
{
    list *head = NULL, *tail = NULL;
    for (int i = 0; i < n; i++) {
        list *node = malloc(sizeof(list));
        node->data = vals[i];
        node->next = NULL;
        if (!head)
            head = node;
        else
            tail->next = node;
        tail = node;
    }
    return head;
}

static inline int slist_to_array(list *head, int *arr, int max)
{
    int i = 0;
    for (list *p = head; p && i < max; p = p->next)
        arr[i++] = p->data;
    return i;
}

static inline int slist_length(list *head)
{
    int n = 0;
    for (list *p = head; p; p = p->next)
        n++;
    return n;
}

static inline void free_slist(list *head)
{
    while (head) {
        list *tmp = head->next;
        free(head);
        head = tmp;
    }
}

#endif /* MERGE_SORT_H */
