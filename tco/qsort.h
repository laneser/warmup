/*
 * qsort.h - Quicksort variants on singly-linked list.
 *
 * Shared node type and helpers used by all four quicksort implementations
 * and by the Unity test harness.
 */
#ifndef QSORT_H
#define QSORT_H

#include <stdlib.h>
#include <string.h>

typedef struct node {
    int value;
    struct node *next;
} node_t;

static inline void list_add(node_t **list, node_t *n)
{
    n->next = *list;
    *list = n;
}

static inline void list_concat(node_t **left, node_t *right)
{
    while (*left)
        left = &(*left)->next;
    *left = right;
}

/* Build a linked list from an int array (preserving order) */
static inline node_t *make_list(const int *values, int n)
{
    node_t *head = NULL;
    for (int i = n - 1; i >= 0; i--) {
        node_t *node = malloc(sizeof(*node));
        node->value = values[i];
        node->next = head;
        head = node;
    }
    return head;
}

/* Convert linked list back to array */
static inline int list_to_array(node_t *head, int *out, int max)
{
    int i = 0;
    while (head && i < max) {
        out[i++] = head->value;
        head = head->next;
    }
    return i;
}

/* Free entire list */
static inline void free_list(node_t *list)
{
    while (list) {
        node_t *tmp = list->next;
        free(list);
        list = tmp;
    }
}

/* Sort function type */
typedef void (*qsort_fn)(node_t **);

/* Declarations for all four variants */
void qsort_original(node_t **list);
void qsort_tco(node_t **list);
void qsort_method2(node_t **list);
void qsort_iterative(node_t **list);

#endif /* QSORT_H */
